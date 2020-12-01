#include "Patcher.h"

EnsMember::EnsMember(QString cName, jack_client_t * jc)
:numIn(0),
numOut(0),
inPorts(QVector<const jack_port_t *>({NULL, NULL})),
outPorts(QVector<const jack_port_t *>({NULL, NULL})),
audio(false),
jClient(jc)
{
  name = cName.section("-",0,0);
  section = cName.section("-",1,1);
  QString jn = "JCS"+name+section;

  cs = new ChannelStrip();

  audio.init((const char *)jn.toUtf8(), cs);
  audio.start();
}

EnsMember::~EnsMember()
{
  audio.stop();
  //delete &audio;
  delete cs;

}

void EnsMember::regPort(const jack_port_t * p){
  QString sn = jack_port_short_name(p);
  int pNumber = (sn.section("_", 1, 1)).toInt();
  if(jack_port_flags(p) & JackPortIsInput){
    if(inPorts.size()<pNumber){
      inPorts.resize(pNumber);
    }
    inPorts[pNumber-1] = p;
    numIn++;
  }
  if(jack_port_flags(p) & JackPortIsOutput){
    if(outPorts.size()<pNumber){
      outPorts.resize(pNumber);
    }
    
    numOut++;
    if (pNumber-1 == 0){
      jack_connect(jClient, jack_port_name(p), jack_port_name(audio.getInputPort(0)));
      outPorts[pNumber-1] = audio.getOutputPort(0);
    }
  }
  qDebug() <<"Add: now "<<numIn <<" inputs and " <<numOut <<" outputs";
}


int EnsMember::deregPort(const jack_port_t *p){
  QString sn = jack_port_short_name(p);
  int pNumber = (sn.section("_", 1, 1)).toInt();
  if(jack_port_flags(p) & JackPortIsInput){
    if(!(inPorts.size()<pNumber)){
      inPorts[pNumber-1] = NULL;
      numIn--;
    }
    /*
    if (pNumber-1 == 0){
      jack_disconnect(jClient, jack_port_name(p), jack_port_name(audio.getInputPort(0)));
    }
    */
  }
  if(jack_port_flags(p) & JackPortIsOutput){
    if(!(outPorts.size()<pNumber)){
      outPorts[pNumber-1] = NULL;
      numOut--;
    }
    
  }
  
  
  qDebug() <<numIn <<" inputs and " <<numOut <<" outputs remain";
  return numIn + numOut;
}

const jack_port_t * EnsMember::inPort(int n){
  if (n < numIn)
    return inPorts[n];
  return NULL;
}

const jack_port_t * EnsMember::outPort(int n){
  if (n < numOut)
    return outPorts[n];
  return NULL;
}