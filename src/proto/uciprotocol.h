#ifndef KNIGHTS_UCIPROTOCOL_H
#define KNIGHTS_UCIPROTOCOL_H

#include <proto/textprotocol.h>

class KProcess;

namespace Knights {

class UciProtocol : public Knights::TextProtocol
{
  
public:
    UciProtocol(QObject* parent = 0);
    virtual ~UciProtocol();

protected:
    virtual bool parseStub(const QString& line);
    virtual void parseLine(const QString& line);

public:
    virtual void declineOffer(const Knights::Offer& offer);
    virtual void acceptOffer(const Knights::Offer& offer);
    virtual void makeOffer(const Knights::Offer& offer);
    virtual void startGame();
    virtual void init();
    virtual void move(const Knights::Move& m);
    
private:
    KProcess* mProcess;
};

}

#endif // KNIGHTS_UCIPROTOCOL_H