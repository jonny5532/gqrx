/* -*- c++ -*- */
/*
 * Copyright 2011 Alexandru Csete OZ9AEC.
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <receiver.h>


receiver::receiver(const std::string input_device, const std::string audio_device)
    : d_audio_rate(48000)
{
    d_tb = gr_make_top_block("gqrx");

    d_fcd_src = fcd_make_source_c(input_device);

    d_audio_sink = audio_make_sink(d_audio_rate, audio_device, true);

    d_tb->connect(d_fcd_src, 0, d_audio_sink, 0);

}

receiver::~receiver()
{
  d_tb->stop();
  d_tb->wait();

  /* FIXME: delete blocks? */
}


void receiver::start()
{
    /* FIXME: Check that flow graph is not running */
    d_tb->start();
}

void receiver::stop()
{
    d_tb->stop();
}


rx_status_t receiver::set_rf_freq(float freq_hz)
{
    return RX_STATUS_OK;
}


rx_status_t receiver::set_rf_gain(float gain_db)
{
    return RX_STATUS_OK;
}


rx_status_t receiver::set_tuning_offset(float offset_hz)
{
    return RX_STATUS_OK;
}


rx_status_t receiver::set_filter_low(float freq_hz)
{
    return RX_STATUS_OK;
}


rx_status_t receiver::set_filter_high(float freq_hz)
{
    return RX_STATUS_OK;
}


rx_status_t receiver::set_demod(rx_demod_t demod)
{
    return RX_STATUS_OK;
}


rx_status_t receiver::set_af_gain(float gain_db)
{
    return RX_STATUS_OK;
}
