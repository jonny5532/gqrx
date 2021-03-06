/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           http://gqrx.dk/
 *
 * Copyright 2011 Alexandru Csete OZ9AEC.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <QMessageBox>
#include <QFileDialog>
#include <cmath>
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/digital/mpsk_receiver_cc.h>
#include <gnuradio/blocks/complex_to_real.h>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdarg.h>
#include "dsp/rx_gendigital.h"

static const int MIN_IN = 1;  /* Mininum number of input streams. */
static const int MAX_IN = 1;  /* Maximum number of input streams. */
static const int MIN_OUT = 0; /* Minimum number of output streams. */
static const int MAX_OUT = 0; /* Maximum number of output streams. */

/*
 * Create a new instance of rx_gendigital and return
 * a boost shared_ptr. This is effectively the public constructor.
 */
rx_gendigital_sptr make_rx_gendigital(double sample_rate)
{
    return gnuradio::get_initial_sptr(new rx_gendigital(sample_rate));
}

// rx_gendigital_store_sptr make_rx_gendigital_store()
// {
//     return gnuradio::get_initial_sptr(new rx_gendigital_store());
// }

#define SNIFFER_BUFSIZE 1000000

rx_gendigital::rx_gendigital(double sample_rate)
    : gr::hier_block2 ("rx_gendigital",
                      gr::io_signature::make (MIN_IN, MAX_IN, sizeof (gr_complex)),
                      gr::io_signature::make (MIN_OUT, MAX_OUT, sizeof (char))),
      d_sample_rate(sample_rate)
{
	float d_gain = 1;
	float d_cutoff_freq = 15000;
	float d_trans_width = 4000;
	
 	d_cutoff_freq = 30000;
 	d_trans_width = 5000;
	
	d_taps = gr::filter::firdes::low_pass(d_gain, sample_rate,
                                 d_cutoff_freq, d_trans_width);
	lpf = gr::filter::fir_filter_ccf::make(1, d_taps);	
	
	sql = gr::analog::simple_squelch_cc::make(-100.0, 0.001);
	
	quadrature_demod = gr::analog::quadrature_demod_cf::make(1);

	locked = 0;
	synced = 0;
	sync_word_bit_length = 32;
	oversampling = 10;
	


	baud_rate = 1600;
	sync_word = 0xA6C6AAAA; //FLEX

	sync_word = 0x7cd215d8; //POCSAG
	baud_rate = 1200;

 	baud_rate = 10000;
 	sync_word = 0xaa;
 	sync_word_bit_length = 8;
	
	output_word = 0;
	output_word_position = 0;
	
	pocsag_buffer_length = 0;
	
	sniffer_rr = make_resampler_ff(float(baud_rate * oversampling) / sample_rate);
	
	sniffer = make_sniffer_f(SNIFFER_BUFSIZE);
	sniffer->set_buffer_size(SNIFFER_BUFSIZE);
	
	wavfile = gr::blocks::wavfile_sink::make("test.wav", 1, (baud_rate * oversampling), 16);

	samplog = new std::ofstream("out.log");
	
	connect(self(), 0, lpf, 0);
	connect(lpf, 0, sql, 0);
	connect(sql, 0, quadrature_demod, 0);
	connect(quadrature_demod, 0, sniffer_rr, 0);
	connect(sniffer_rr, 0, sniffer, 0);
	connect(sniffer_rr, 0, wavfile, 0);
}

rx_gendigital::~rx_gendigital ()
{

}

void rx_gendigital::set_sync_word(std::string &hex) 
{
	unsigned int word_length = 0;
	unsigned long long word = 0;
	
	for(std::string::iterator it = hex.begin(); it != hex.end(); ++it) {
		unsigned int c = 0;
		if(*it >= '0' && *it <= '9') {
			c = (*it)-'0';
		} else if(*it >= 'A' && *it <= 'F') {
			c = ((*it)-'A')+10;
		} else if(*it >= 'a' && *it <= 'f') {
			c = ((*it)-'a')+10;
		} else {
			return; //invalid!
		}
		word = (word << 4) | (c&0xf);
		word_length += 4;
	}
	sync_word = word;
	sync_word_bit_length = word_length;
}

void rx_gendigital::set_baud_rate(unsigned int br)
{
	baud_rate = br;
	sniffer_rr->set_rate(float(baud_rate * oversampling) / d_sample_rate);
	locked = 0;
}

void rx_gendigital::set_squelch(double v)
{
	sql->set_threshold(v);
}

void rx_gendigital::set_decode_format(std::string &format)
{
	decode_format = format;
}

/* so, interesting parameters
 * 
 * - baud rate
 * - squelch?
 * - synchronisation code? probably just done in decoder
 * 
 * */

void rx_gendigital::process ()
{
	float buf[SNIFFER_BUFSIZE];
	unsigned int num;

	sniffer->get_samples(&buf[0], num); //remember samples are inverted
	
	unsigned int middle_bit = oversampling/2;
		
	for(unsigned int i=0;i<num;i++) {
		if(static_bits>30) { //if we haven't had a transition in 30 bits, assume we're out of sync
			locked = 0;
			synced = 0;
		}
		
		if(locked==0) {
			if(i>0 && buf[i]>=0 && buf[i-1]<0) { // 1->0 transition
				locked = 1;
				current_bit = 0;
				static_bits = 0;
				nudge = 0;
				*samplog << "\n\n";
			} else {
				continue;
			}
		}
		
		if(current_bit==0) {
			*samplog << '_';
		}
		
		if(buf[i]<0) {
			*samplog << '1';
		} else {
			*samplog << '0';
		}
		
		if(current_bit==0) {
			bit_sum = 0.0f;
		} else if(current_bit>0 && current_bit<(oversampling-1)) {
			bit_sum -= float(buf[i]);
		} else if(current_bit == (oversampling-1)) {
			static_bits++;
			if(bit_sum>0) {
				if(output==0) static_bits = 0;
				output = 1;
				std::cout << "1";
			} else {
				if(output==1) static_bits = 0;
				output = 0;
				std::cout << "0";
			}
			
			//shift latest bit onto start of output_word
			output_word |= ((output&1)<<(31-output_word_position));
			output_word_position++;
			
			//shift latest bit onto start of rolling sync word buffer
			rolling_sync_word_buffer = ( (rolling_sync_word_buffer << 1) & ( (((unsigned long long)1)<<sync_word_bit_length)-1 ) ) | output;
			
			if(rolling_sync_word_buffer==sync_word) {
				if(decode_format=="POCSAG") {
				} else {
					output_buffer << "\n\n";
				}
				//std::cout << "\nsync word found!\n";
				output_word = 0;
				output_word_position = 0;
				synced = 1;
			}
			
			if(output_word_position==32 && synced==1) {
				//output the data!
				if(decode_format=="POCSAG") {
					if(output_word & 0x80000000) { // is a message word
						pocsag_in_message = 1;
						pocsag_buffer = (pocsag_buffer<<20) | ((output_word>>11)&0xfffff);
						pocsag_buffer_length += 20;
						while(pocsag_buffer_length>=7) {
							char inc = char((pocsag_buffer>>(pocsag_buffer_length-7))&0x7f);
							char outc = 0;
							for(int ci=0;ci<7;ci++) {
								outc = (outc<<1) | ((inc>>ci)&1);
							}
							if(outc<' ' || outc>'z') outc = ' ';
							output_buffer << outc;
							pocsag_buffer_length -= 7;
						}
					} else {
						pocsag_buffer_length = 0;
						if(pocsag_in_message) {
							output_buffer << "\n\n";
						}
						pocsag_in_message = 0;
					}
				} else {
					char sbuf[32];
					snprintf(sbuf, sizeof(sbuf), "%02x %02x %02x %02x ", (output_word>>24), (output_word>>16)&0xff, (output_word>>8)&0xff, output_word&0xff);
					output_buffer << sbuf;
				}
				output_word = 0;
				output_word_position = 0;
			}
			
			
			if(i>=middle_bit && ((output==1 && buf[i-middle_bit]>0) || (output==0 && buf[i-middle_bit]<0)) ) {
				// first edge came late!
				nudge += 1;
				if(nudge>4) {
					i += 1;
					std::cout << ">";
					*samplog << ">";
					nudge = 0;
				}
			}
		}
		

		if(current_bit==(oversampling-1) && ((output==1 && buf[i]>0) || (output==0 && buf[i]<0)) ) {
			// next edge came early!
//			i -= 1;
			nudge -= 1;
			if(nudge<-4) {
				i -= 1;
				std::cout << "<";
				*samplog << "<";
				nudge = 0;
			}
		}
		
		current_bit++;		
		if(current_bit>=oversampling) current_bit = 0;
	
// 		if(buf[i]>0) std::cout << "1";
// 		else std::cout << "0";
	}
}

void rx_gendigital::get_output(std::string &outbuff)
{
	outbuff = output_buffer.str();
	output_buffer.clear();
	output_buffer.str(std::string());
}

// rx_gendigital_store::rx_gendigital_store() : gr::block ("rx_gendigital_store",
//                                 gr::io_signature::make (0, 0, 0),
//                                 gr::io_signature::make (0, 0, 0))
// {
//         message_port_register_in(pmt::mp("store"));
//         set_msg_handler(pmt::mp("store"), boost::bind(&rx_gendigital_store::store, this, _1));
//         d_messages.set_capacity(100);
// }
// 
// rx_gendigital_store::~rx_gendigital_store ()
// {
// 
// }
// 
// void rx_gendigital_store::store(pmt::pmt_t msg)
// {
//     boost::mutex::scoped_lock lock(d_mutex);
//     d_messages.push_back(msg);
// 
// }
// 
// void rx_gendigital_store::get_message(std::string &out, int &type)
// {
//     boost::mutex::scoped_lock lock(d_mutex);
// 
//     if (d_messages.size()>0) {
//         pmt::pmt_t msg=d_messages.front();
//         type=pmt::to_long(pmt::tuple_ref(msg,0));
//         out=pmt::symbol_to_string(pmt::tuple_ref(msg,1));
//         d_messages.pop_front();
//     } else {
//         type=-1;
//     }
// }
