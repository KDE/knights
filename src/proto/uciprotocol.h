#ifndef KNIGHTS_UCIPROTOCOL_H
#define KNIGHTS_UCIPROTOCOL_H

#include "proto/computerprotocol.h"
#include <QStack>

class KProcess;

namespace Knights {

class UciProtocol : public ComputerProtocol
{
  
public:
    UciProtocol(QObject* parent = 0);
    virtual ~UciProtocol();

protected:
    virtual bool parseStub(const QString& line);
    virtual void parseLine(const QString& line);

public:
    virtual Features supportedFeatures();
    virtual void declineOffer(const Knights::Offer& offer);
    virtual void acceptOffer(const Knights::Offer& offer);
    virtual void makeOffer(const Knights::Offer& offer);
    virtual void startGame();
    virtual void init();
    virtual void move(const Knights::Move& m);
    virtual void setDifficulty(int level);
    
private slots:
    void changeCurrentTime(Knights::Color color, const QTime& time);
    
private:
    QStack<Move> mMoveHistory;
    int mWhiteTime;
    int mBlackTime;
    Move mPonderMove;
    int mDifficulty;
};

}

#endif // KNIGHTS_UCIPROTOCOL_H
