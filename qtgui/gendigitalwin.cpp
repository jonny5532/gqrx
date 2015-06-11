/* -*- c++ -*- */
/*

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
#include <QFile>
#include <QDir>
#include <QDebug>
#include "gendigitalwin.h"
#include "ui_gendigitalwin.h"

#include <iostream>

GenDigitalWin::GenDigitalWin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GenDigitalWin)
{
    ui->setupUi(this);

    /* select font for text viewer */
#ifdef Q_OS_MAC
    ui->textView->setFont(QFont("Monaco", 12));
#else
    ui->textView->setFont(QFont("Monospace", 11));
#endif

    /* Add right-aligned info button */
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//     ui->toolBar->addWidget(spacer);
//     ui->toolBar->addAction(ui->actionInfo);

    /* AFSK1200 decoder */
//     decoder = new CAfsk12(this);
//     connect(decoder, SIGNAL(newMessage(QString)), ui->textView, SLOT(appendPlainText(QString)));
	connect(ui->baudRate, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_baudRate_changed()));
	connect(ui->syncWord, SIGNAL(textChanged(QString)), this, SLOT(on_syncWord_changed()));
}

GenDigitalWin::~GenDigitalWin()
{
    qDebug() << "AFSK1200 decoder destroyed.";

    //delete decoder;
    delete ui;
}


void GenDigitalWin::received_message(std::string buf) 
{
	ui->textView->moveCursor(QTextCursor::End);
	ui->textView->insertPlainText(QString::fromStdString(buf));
	ui->textView->moveCursor(QTextCursor::End);
	//ui->textView->setPlainText(ui->textView->toPlainText() + QString::fromStdString(buf));
	//ui->textView->plainText += QString::fromStdString(buf);
}


/*! \brief Catch window close events and emit signal so that main application can destroy us. */
void GenDigitalWin::closeEvent(QCloseEvent *ev)
{
    Q_UNUSED(ev);

    emit windowClosed();
}

void GenDigitalWin::on_baudRate_changed()
{
	emit baudRateChanged(ui->baudRate->currentText().toInt());
	//std::cout << "changed! " << ui->baudRate->currentText().toInt() << "\n";
}

void GenDigitalWin::on_syncWord_changed()
{
	emit syncWordChanged(ui->syncWord->text().toStdString());
	//std::cout << "changed! " << ui->syncWord->text() << "\n";
}



/*
*! \brief User clicked on the Clear button. *
void GenDigitalWin::on_actionClear_triggered()
{
    ui->textView->clear();
}


*! \brief User clicked on the Save button. *
void Afsk1200Win::on_actionSave_triggered()
{
    /* empty text view has blockCount = 1 *
    if (ui->textView->blockCount() < 2) {
        QMessageBox::warning(this, tr("Gqrx error"), tr("Nothing to save."),
                             QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    QDir::homePath(),
                                                    tr("Text Files (*.txt)"));

    if (fileName.isEmpty()) {
        qDebug() << "Save cancelled by user";
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error creating file: " << fileName;
        return;
    }

    QTextStream out(&file);
    out << ui->textView->toPlainText();
    file.close();
}


*! \brief User clicked Info button. *
void Afsk1200Win::on_actionInfo_triggered()
{
    QMessageBox::about(this, tr("About AFSK1200 Decoder"),
                       tr("<p>Gqrx AFSK1200 Decoder %1</p>"
                          "<p>The Gqrx AFSK1200 decoder taps directly into the SDR signal path "
                          "eliminating the need to mess with virtual or real audio cables. "
                          "It can decode AX.25 packets and displays the decoded packets in a text view.</p>"
                          "<p>The decoder is based on Qtmm, which is avaialble for Linux, Mac and Windows "
                          "at <a href='http://qtmm.sf.net/'>http://qtmm.sf.net</a>.</p>"
                          ).arg(VERSION));

}*/
