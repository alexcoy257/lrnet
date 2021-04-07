#ifndef LRNET_MEMBER_H
#define LRNET_MEMBER_H
#include <QObject>
#include <QSet>
#include "auth_types.h"
#include "JackTripWorker.h"
#include <jack/jack.h>


//namespace faustNS{

#define UI F_UI
#include "faust/gui/ControlUI.h"
#include "faust/dsp/dsp.h"
#include "faust/gui/meta.h"
#include "faust/audio/jack-dsp.h"


#include "channelStrip.h"
#undef UI
//}


//using faustNS::ChannelStrip, faustNS::jackaudio, faustNS::ControlUI;


class Roster;

class Member: public QObject{
    Q_OBJECT
    class csControlPair;

public:
    typedef enum {
        COMP_BYPASS = 0,
        COMP_RATIO,
        COMP_THRESHOLD,
        COMP_ATTACK,
        COMP_RELEASE,
        COMP_MAKEUP,
        GROUP_GAIN,
        INDIV_GAIN,
        MUTE
    }CSControlsE;
    typedef enum {
        COMP_BYPASSE = 0
    }CSReadsE;
    typedef quint64 serial_t;

    void setControl(int out, float val);
    const float * getCurrentControls();

private:
    session_id_t s_id;
    static serial_t currentSerial;
    serial_t serial;
    QString netid;
    QString name;
    QString section;
    Roster * mRoster;
    JackTripWorker * assocThread = NULL;
    int mPort;
    jack_port_t * fromPorts[2] = {NULL, NULL};
    jack_port_t * toPorts[2] = {NULL, NULL};
    jack_port_t * broadcastPorts[2] = {NULL, NULL};

    QTimer mSaveControlTimer;

    int mNumChannels = 1;
    std::vector<std::unique_ptr<csControlPair>> cses;
    ChannelStrip * cs ;
    ControlUI * ui;
    jackaudio * audio;

    float currentControlValues[9] = {0.,2.,-24.,15.,40.,2.,0,0, 1};
    void connectChannelStrip();

    const QSet<CSControlsE> savedControls = QSet<CSControlsE>({COMP_RATIO,
                                                         COMP_THRESHOLD,
                                                         COMP_ATTACK,
                                                         COMP_RELEASE,
                                                         COMP_MAKEUP,
                                                         INDIV_GAIN});

public:
    static int constexpr numControlValues = 9;
    explicit Member(QObject *parent = nullptr);
    explicit Member(QString & netid, session_id_t s_id, Roster * roster,  QObject *parent = nullptr);
    ~Member();

    session_id_t getSessionID(){return s_id;}
    uint64_t getSerialID(){return serial;}
    QString & getNetID(){return netid;}
    QString & getName(){return name;}
    QString & getSection(){return section;}
    void setName(QString & nname);
    void setSection(QString & nsection);
    void setThread(JackTripWorker * thread){assocThread=thread;}
    void setNumChannels(int n);
    int getNumChannels(){return mNumChannels;}
    void setPort(int port){mPort = port;}
    int getPort(){return mPort;}
    audioPortHandle_t getAudioInputPort(int n);
    audioPortHandle_t getAudioOutputPort(int n);
    JackTripWorker * getThread(){return assocThread;}
    void resetThread();
    void setLoopback(bool lb);

signals:
    void readyToFan(Member * m);
    void saveMemberControls(Member * m);
};
#endif // LRNET_MEMBER_H
