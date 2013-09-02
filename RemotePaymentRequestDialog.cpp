#include "RemotePaymentRequestDialog.h"
#include "ui_RemotePaymentRequestDialog.h"

#include "Currency.h"
#include "p3ZeroReserverRS.h"
#include "ZeroReservePlugin.h"

#include "util/radix64.h"

#include <openssl/sha.h>


RemotePaymentRequestDialog::RemotePaymentRequestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemotePaymentRequestDialog)
{
    ui->setupUi(this);
    int index = 0;
    while(Currency::currencyNames[ index ]){
        ui->currencySelector->addItem( Currency::currencyNames[ index ] );
        index++;
    }

    ui->amount->setValidator( new QDoubleValidator( 1E-8, 1E12, 0) );

    connect( ui->currencySelector, SIGNAL( currentIndexChanged( QString ) ), this, SLOT( currencySelected( QString ) ) );
    connect( ui->amount, SIGNAL( textEdited( QString ) ), this, SLOT( amountEntered( QString ) ) );
    connect( ui->buttonBox, SIGNAL( accepted() ), this, SLOT( sendRequest() ) );
}


RemotePaymentRequestDialog::~RemotePaymentRequestDialog()
{
    delete ui;
}


void RemotePaymentRequestDialog::currencySelected( QString currency )
{
    QString amount = ui->amount->text();
    if( !amount.isEmpty() ){
        ui->payAddress->setText( getPayAddress( currency, amount ) );
    }
}

void RemotePaymentRequestDialog::amountEntered( QString amount )
{
    QString currency =  QString::fromStdString( Currency::currencySymbols[ Currency::getCurrencyByName( ui->currencySelector->currentText().toStdString() ) ] );
    ui->payAddress->setText( getPayAddress( currency, amount ) );
}


QString RemotePaymentRequestDialog::getPayAddress( QString amount, QString currency )
{
    unsigned char md[ SHA256_DIGEST_LENGTH ];
    p3ZeroReserveRS * p3zr = static_cast< p3ZeroReserveRS* >( g_ZeroReservePlugin->rs_pqi_service() );
    // FIXME: the getOwnId() must be replaced by a secret - else the originator of the order can be calculated
    // FIXME: by friends and friends of friends
    QString data = QString::fromStdString( p3zr->getOwnId() ) + amount + currency + QString::number( rand() );
    int length = data.length();
    unsigned char * buff = new unsigned char[ length ];
    memcpy( buff, data.toAscii(), length );
    SHA256( buff, length, md);
    std::string payAddress;
    Radix64::encode( (const char*)md, SHA256_DIGEST_LENGTH, payAddress );
    delete [] buff;
    return QString::fromStdString( payAddress );
}


void RemotePaymentRequestDialog::sendRequest()
{

}
