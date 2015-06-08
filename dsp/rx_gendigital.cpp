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
#include <iostream>
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
	float d_cutoff_freq = 6000;
	float d_trans_width = 1000;
	
	d_taps = gr::filter::firdes::low_pass(d_gain, sample_rate,
                                 d_cutoff_freq, d_trans_width);
	lpf = gr::filter::fir_filter_ccf::make(1, d_taps);	
	
	quadrature_demod = gr::analog::quadrature_demod_cf::make(1);
	
// 	std::cout << "samplerate is " << (sample_rate) << "\n";
// 	
// 	std::cout << "scalefactor is " << ((float)sample_rate/(float)1200) << "\n";
	
	sniffer_rr = make_resampler_ff(4800.0f / sample_rate);
	
	sniffer = make_sniffer_f(SNIFFER_BUFSIZE);
	sniffer->set_buffer_size(SNIFFER_BUFSIZE);
	
	wavfile = gr::blocks::wavfile_sink::make("test.wav", 1, 4800, 16);
//    d_taps2 = gr::filter::firdes::low_pass(2500.0, d_sample_rate, 2400, 2000);

	connect(self(), 0, lpf, 0);
	connect(lpf, 0, quadrature_demod, 0);
	connect(quadrature_demod, 0, sniffer_rr, 0);
	connect(sniffer_rr, 0, sniffer, 0);
	connect(sniffer_rr, 0, wavfile, 0);
// 	connect(quadrature_demod, 0, sniffer, 0);
// 	connect(self(), 0, lpf, 0);
	
//     f_fxff = gr::filter::freq_xlating_fir_filter_fcf::make(5.0, d_taps2, 57000, d_sample_rate);
// 
//     f_rrcf = gr::filter::firdes::root_raised_cosine(1, sample_rate/5, 2375, 1, 100);
//     d_bpf2 = gr::filter::fir_filter_ccf::make(1, f_rrcf);
// 
//     d_mpsk = gr::digital::mpsk_receiver_cc::make(2, 0, 1*M_PI/100.0, -0.06, 0.06, 0.5, 0.05, sample_rate/5/2375.0, 0.001, 0.005);
// 
// 
//     b_ctr = gr::blocks::complex_to_real::make(1);
// 
//     d_bs = gr::digital::binary_slicer_fb::make();
// 
//     b_koin = gr::blocks::keep_one_in_n::make(sizeof(unsigned char), 2);
// 
//     d_ddbb = gr::digital::diff_decoder_bb::make(2);

//     rds_decoder = gr::rds::decoder::make(0, 0);
//     rds_parser = gr::rds::parser::make(1, 0);

    /* connect filter */
//     connect(self(), 0, f_fxff, 0);
//     connect(f_fxff, 0, d_bpf2, 0);
//     connect(d_bpf2, 0, d_mpsk, 0);
//     connect(d_mpsk, 0, b_ctr, 0);
//     connect(b_ctr, 0, d_bs, 0);
//     connect(d_bs, 0, b_koin, 0);
//     connect(b_koin, 0, d_ddbb, 0);
//     connect(d_ddbb, 0, self(), 0);
	//connect(self(), 0, self(), 0);
}

rx_gendigital::~rx_gendigital ()
{

}

void rx_gendigital::process ()
{
	float buff[SNIFFER_BUFSIZE];
	unsigned int num;
	
	sniffer->get_samples(&buff[0], num);
	
	if(num>0) {
		float min = buff[0];
		float max = buff[0];
		
		
		for(unsigned int i=0;i<num;i++) {
			if(buff[i]>max) max = buff[i];
			else if(buff[i]<min) min = buff[i];
		
			if(buff[i]>0) std::cout << "1";
			else std::cout << "0";
// 			std::cout << buff[i] <<" ";
		}
		std::cout << "\n";
		
		//std::cout << "range from " << min << "\t to " << max << "\n";
		
		//std::cout << "got " << num << " samples\n";
	}
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
