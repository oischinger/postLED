#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#include "led-matrix.h"

#include "mongoose/mongoose.h"

#include "ScrollingTextGenerator.h"
#include "VolumeBars.h"
#include "ListUI.h"
#include "Alarm.h"

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

static const char *s_address = "127.0.0.1:1883";
static const char *s_user_name = nullptr;
static const char *s_password = nullptr;
static const char *s_topic = "/postled/#";
static struct mg_mqtt_topic_expression s_topic_expr = {nullptr, 0};

static ScrollingTextGenerator* s_pTextScrollGenerator;
static VolumeBars* s_pVolumeBars;
static ListUI* s_pListUi;
static Alarm* s_pAlarm;
static bool sbTextScrollGeneratorInterrupted = false;

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options] <text>\n", progname);
  fprintf(stderr, "postled MQTT LED Matrix client\n");
  fprintf(stderr, "Options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  fprintf(stderr,
          "\t-h <hostname>     : MQTT broker hostname.\n"
          "\t-u <user>         : MQTT broker username.\n"
          "\t-p <password>     : MQTT broker passowrd.\n"
          "\t-t <topic>        : MQTT topic.\n"
          );
  return 1;
}

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;
  (void) nc;

  if (ev != MG_EV_POLL) printf("USER HANDLER GOT EVENT %d\n", ev);

  switch (ev) {
    case MG_EV_CONNECT: {
      struct mg_send_mqtt_handshake_opts opts;
      memset(&opts, 0, sizeof(opts));
      opts.user_name = s_user_name;
      opts.password = s_password;

      mg_set_protocol_mqtt(nc);
      mg_send_mqtt_handshake_opt(nc, "postled", opts);
      break;
    }
    case MG_EV_MQTT_CONNACK:
      if (msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED) {
        printf("Got mqtt connection error: %d\n", msg->connack_ret_code);
        exit(1);
      }
      s_topic_expr.topic = s_topic;
      printf("Subscribing to '%s'\n", s_topic);
      mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);
      break;
    case MG_EV_MQTT_PUBACK:
      printf("Message publishing acknowledged (msg_id: %d)\n", msg->message_id);
      break;
    case MG_EV_MQTT_SUBACK:
      printf("Subscription acknowledged\n");
      break;
    case MG_EV_MQTT_PUBLISH: {
#if 0
        char hex[1024] = {0};
        mg_hexdump(nc->recv_mbuf.buf, msg->payload.len, hex, sizeof(hex));
        printf("Got incoming message %.*s:\n%s", (int)msg->topic.len, msg->topic.p, hex);
#else
      char* strTopic = strndup(msg->topic.p, msg->topic.len);
      char* strPayload = strndup(msg->payload.p, msg->payload.len);
      printf("Got incoming message %.*s: %.*s\n", (int) msg->topic.len,
             msg->topic.p, (int) msg->payload.len, strPayload);
      
      
      if (s_pTextScrollGenerator->isRunning() && (strcmp("/postled/mediaplayer/volume", strTopic) == 0))
          sbTextScrollGeneratorInterrupted = true;
      s_pTextScrollGenerator->Stop();
      s_pTextScrollGenerator->WaitStopped();
      s_pVolumeBars->Stop();
      s_pVolumeBars->WaitStopped();
      s_pListUi->Stop();
      s_pListUi->WaitStopped();
      s_pAlarm->Stop();
      s_pAlarm->WaitStopped();
      
      if (strcmp("/postled/mediaplayer/currentsong", strTopic) == 0)
      {
          s_pTextScrollGenerator->setText(strPayload);
          s_pTextScrollGenerator->setMusic(true);
          s_pTextScrollGenerator->setLoops(3);
          s_pTextScrollGenerator->Start();
      }
      else if (strcmp("/postled/showlist", strTopic) == 0)
      {
          s_pListUi->setData(strPayload);
          s_pListUi->Start();
      }
      else if (strcmp("/postled/alarm", strTopic) == 0)
      {
          s_pAlarm->Start();
      }
      else if (strcmp("/postled/mediaplayer/volume", strTopic) == 0)
      {
          int volume = atoi(strPayload);
          s_pVolumeBars->setVolume(volume);
          s_pVolumeBars->Start();
      }
      else if (strcmp("/postled/warning", strTopic) == 0)
      {
          s_pTextScrollGenerator->setText(strPayload);
          s_pTextScrollGenerator->setMusic(false);
          s_pTextScrollGenerator->setLoops(30);
          s_pTextScrollGenerator->Start();
      }
      fflush(stdout);
#endif
      break;
    }
    case MG_EV_CLOSE:
      printf("Connection closed. Reconnecting in 1 sec...\n");
      sleep(1);
      
      // Connect MQTT
      if (mg_connect(nc->mgr, s_address, ev_handler) == NULL) {
        fprintf(stderr, "mg_connect(%s) failed\n", s_address);
        sleep(1);
      }
  }
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

int main(int argc, char *argv[]) {
  // Signal handler to exit gracefully
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // MQTT client init
  struct mg_mgr mgr;
  int i;
  mg_mgr_init(&mgr, nullptr);

  /* Parse MQTT command line arguments */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      s_address = argv[++i];
    } else if (strcmp(argv[i], "-u") == 0) {
      s_user_name = argv[++i];
    } else if (strcmp(argv[i], "-t") == 0) {
      s_topic = argv[++i];
    } else if (strcmp(argv[i], "-p") == 0) {
      s_password = argv[++i];
    }
  }
  
  /* Parse RGBMatrix command line opts */
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }
  
  RGBMatrix *matrix = rgb_matrix::CreateMatrixFromOptions(matrix_options,
                                                          runtime_opt);
  s_pTextScrollGenerator = new ScrollingTextGenerator(matrix);
  s_pVolumeBars = new VolumeBars(matrix);
  s_pListUi = new ListUI(matrix);
  s_pAlarm = new Alarm(matrix); 

  // Connect MQTT
  if (mg_connect(&mgr, s_address, ev_handler) == nullptr) {
    fprintf(stderr, "mg_connect(%s) failed\n", s_address);
    sleep(1);
  }

  // Main LOOP
  for (;;) {
    if (sbTextScrollGeneratorInterrupted &&
        !s_pTextScrollGenerator->isRunning() &&
        !s_pVolumeBars->isRunning() &&
        !s_pListUi->isRunning())
    {
        sbTextScrollGeneratorInterrupted = false;
        s_pTextScrollGenerator->WaitStopped();
        s_pTextScrollGenerator->Start();
    }
    mg_mgr_poll(&mgr, 1000);
    if (interrupt_received)
    {
      s_pTextScrollGenerator->Stop();
      s_pVolumeBars->Stop();
      s_pListUi->Stop();
      s_pAlarm->Stop();
      exit(EXIT_FAILURE);
    }
  }
}
