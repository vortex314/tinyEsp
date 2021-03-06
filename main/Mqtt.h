#ifndef MQTT_ABSTRACT_H
#define MQTT_ABSTRACT_H
#include <NanoAkka.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>
typedef struct MqttMessage
{
    NanoString topic;
    NanoString message;
} MqttMessage;
//____________________________________________________________________________________________________________
//
template <class T>
class ToMqtt : public LambdaFlow<T, MqttMessage>
{
    NanoString _name;

public:
    ToMqtt(NanoString name)
        : LambdaFlow<T, MqttMessage>([&](MqttMessage &msg, const T &event) {
              NanoString s;
              DynamicJsonDocument doc(100);
              JsonVariant variant = doc.to<JsonVariant>();
              variant.set(event);
              serializeJson(doc, s);
              msg = {_name, s};
              return 0;
          }),
          _name(name) {}
    void request(){};
};
//_______________________________________________________________________________________________________________
//
template <class T>
class FromMqtt : public LambdaFlow<MqttMessage, T>
{
    NanoString _name;

public:
    FromMqtt(NanoString name)
        : LambdaFlow<MqttMessage, T>([&](T &t, const MqttMessage &mqttMessage) {
              //       INFO(" '%s' <> '%s'",mqttMessage.topic.c_str(),_name.c_str());
              if (mqttMessage.topic != _name)
              {
                  return EINVAL;
              }
              DynamicJsonDocument doc(1000);
              auto error = deserializeJson(doc, mqttMessage.message);
              if (error)
              {
                  WARN(" failed JSON parsing '%s...' : '%s' ", mqttMessage.message.substr(0,100).c_str(), error.c_str());
                  return ENODATA;
              }
              JsonVariant variant = doc.as<JsonVariant>();
              if (variant.isNull())
              {
                  WARN(" is not a JSON variant '%s...' ", mqttMessage.message.substr(0,100).c_str());
                  return ENODATA;
              }
              if (variant.is<T>() == false)
              {
                  WARN(" message '%s...' JSON type doesn't match.", mqttMessage.message.substr(0,100).c_str());
                  return ENODATA;
              }
              t = variant.as<T>();
              return 0;
              // emit doesn't work as such
              // https://stackoverflow.com/questions/9941987/there-are-no-arguments-that-depend-on-a-template-parameter
          }),
          _name(name){};
    void request(){};
};
//____________________________________________________________________________________________________________
//
template <class T>
class MqttFlow : public Flow<T, T>
{
public:
    ToMqtt<T> toMqtt;
    FromMqtt<T> fromMqtt;
    MqttFlow(const char *topic) : toMqtt(topic), fromMqtt(topic){
                                                     //       INFO(" created MqttFlow : %s ",topic);
                                                 };
    void request()
    {
        fromMqtt.request();
    };
    void on(const T &t)
    {
        toMqtt.on(t);
    }
    void subscribe(Subscriber<T> *tl)
    {
        fromMqtt.subscribe(tl);
    };
};
//____________________________________________________________________________________________________________
//
class Mqtt : public Actor
{
public:
    QueueFlow<MqttMessage, 5> incoming;
    Sink<MqttMessage, 10> outgoing;
    ValueSource<bool> connected;
    TimerSource keepAliveTimer;
    Mqtt(Thread &thr) : Actor(thr){};
    ~Mqtt(){};
    void init();
    template <class T>
    Subscriber<T> &toTopic(const char *name)
    {
        auto flow = new ToMqtt<T>(name);
        *flow >> outgoing;
        return *flow;
    }
    template <class T>
    Source<T> &fromTopic(const char *name)
    {
        auto newSource = new FromMqtt<T>(name);
        incoming >> *newSource;
        return *newSource;
    }
    template <class T>
    Flow<T, T> &topic(const char *name)
    {
        auto flow = new MqttFlow<T>(name);
        incoming >> flow->fromMqtt;
        flow->toMqtt >> outgoing;
        return *flow;
    }
};
#endif
