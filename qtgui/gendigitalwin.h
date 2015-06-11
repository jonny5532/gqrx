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
#ifndef GENDIGITALWIN_H
#define GENDIGITALWIN_H

#include <QMainWindow>
#include <QVarLengthArray>



namespace Ui {
    class GenDigitalWin;
}


/*! \brief Generic Digital decoder window. */
class GenDigitalWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit GenDigitalWin(QWidget *parent = 0);
    ~GenDigitalWin();
	void received_message(std::string buf);
//    void process_samples(float *buffer, int length);

protected:
    void closeEvent(QCloseEvent *ev);

signals:
    void windowClosed();  /*! Signal we emit when window is closed. */
	void baudRateChanged(unsigned int baud_rate);
	void syncWordChanged(std::string sync_word);

private slots:
//     void on_actionClear_triggered();
//     void on_actionSave_triggered();
//     void on_actionInfo_triggered();
	void on_baudRate_changed();
	void on_syncWord_changed();
	
	
private:
    Ui::GenDigitalWin *ui;  /*! Qt Designer form. */

//    CAfsk12 *decoder;     /*! The AFSK1200 decoder object. */

//    QVarLengthArray<float, 16384> tmpbuf;   /*! Needed to remember "overlap" smples. */
};

#endif // GENDIGITALWIN_H
