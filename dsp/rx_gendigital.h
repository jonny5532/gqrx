/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           http://gqrx.dk/
 *
 * Copyright 2015 Alexandru Csete OZ9AEC.
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
#ifndef RX_GENDIGITAL_H
#define RX_GENDIGITAL_H

#include <gnuradio/hier_block2.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/analog/simple_squelch_cc.h>
#include <gnuradio/filter/fir_filter_ccc.h>
#include <gnuradio/filter/fir_filter_ccf.h>
#include <gnuradio/filter/fir_filter_fff.h>
#include <gnuradio/filter/freq_xlating_fir_filter_fcf.h>
#include <gnuradio/filter/freq_xlating_fir_filter_ccc.h>
#include <gnuradio/filter/freq_xlating_fir_filter_ccf.h>
#include <gnuradio/digital/mpsk_receiver_cc.h>
#include <gnuradio/blocks/complex_to_real.h>
#include <gnuradio/blocks/keep_one_in_n.h>
#include <gnuradio/blocks/wavfile_sink.h>
#include <gnuradio/digital/binary_slicer_fb.h>
#include <gnuradio/digital/diff_decoder_bb.h>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/udp_sink.h>
#include <gnuradio/blocks/message_debug.h>
#include <boost/circular_buffer.hpp>
#include "dsp/resampler_xx.h"
#include "dsp/sniffer_f.h"

class rx_gendigital;
//class rx_rds_store;

typedef boost::shared_ptr<rx_gendigital> rx_gendigital_sptr;
//typedef boost::shared_ptr<rx_rds_store> rx_rds_store_sptr;


rx_gendigital_sptr make_rx_gendigital(double sample_rate);

//rx_gendigital_store_sptr make_rx_gendigital_store();

// class rx_rds_store : public gr::block
// {
// public:
//     rx_rds_store();
//     ~rx_rds_store();
// 
//     void get_message(std::string &out, int &type);
// 
// private:
//     void store(pmt::pmt_t msg);
// 
//     boost::mutex d_mutex;
//     boost::circular_buffer<pmt::pmt_t> d_messages;
// 
// };

class rx_gendigital : public gr::hier_block2
{

public:
    rx_gendigital(double sample_rate=96000.0);
    ~rx_gendigital();

    void set_param(double low, double high, double trans_width);
	void process();

private:
	gr::analog::simple_squelch_cc::sptr sql;
	
	gr::filter::fir_filter_ccf::sptr lpf;
	std::vector<float> d_taps;	
	
	gr::analog::quadrature_demod_cf::sptr quadrature_demod;
	
	gr::blocks::wavfile_sink::sptr wavfile;
	std::ofstream *samplog;
	
//     std::vector<gr_complex> d_taps;
//     std::vector<float> d_taps2;
//     gr::filter::fir_filter_ccc::sptr  d_bpf;
//     gr::filter::fir_filter_ccf::sptr  d_bpf2;
//     gr::filter::freq_xlating_fir_filter_fcf::sptr f_fxff;
//     gr::filter::freq_xlating_fir_filter_ccf::sptr f_fxff_ccf;
//     std::vector<float> f_rrcf;
//     gr::digital::mpsk_receiver_cc::sptr d_mpsk;
//     gr::blocks::complex_to_real::sptr b_ctr;
//     gr::digital::binary_slicer_fb::sptr d_bs;
//     gr::blocks::keep_one_in_n::sptr b_koin;
//     gr::digital::diff_decoder_bb::sptr d_ddbb;
// //     gr::rds::decoder::sptr rds_decoder;
// //     gr::rds::parser::sptr rds_parser;
//     gr::blocks::udp_sink::sptr udp_sink1;
//     gr::blocks::udp_sink::sptr udp_sink2;
//     gr::blocks::udp_sink::sptr udp_sink3;
//     gr::blocks::udp_sink::sptr udp_sink4;
	sniffer_f_sptr    sniffer;    /*!< Sample sniffer for data decoders. */
    resampler_ff_sptr sniffer_rr; /*!< Sniffer resampler. */

    
    double d_sample_rate;
	unsigned long long sync_word;
	unsigned int sync_word_bit_length;
	unsigned int baud_rate;
	unsigned int oversampling;
	
	unsigned int locked;
	unsigned int synced;
	unsigned int current_bit;
	float bit_sum;
	unsigned int output;
	unsigned int output_word;
	unsigned int output_word_position;
	int nudge;
	unsigned int static_bits;
	unsigned long long rolling_sync_word_buffer;
	
};


#endif // RX_GENDIGITAL_H
