#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#define ADDRESS     "tcp://210.246.215.31:1883"  // Broker address
#define CLIENTID    "C_MQTT_Publisher"
#define TOPIC       "test/topic"
#define PAYLOAD     "Hello MQTT from C!"
#define QOS         0
#define TIMEOUT     10000L

// Your username and password for authentication
#define USERNAME    "tgr"
#define PASSWORD    "tgr18"

int main() {
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Set the connection options, including username and password
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = USERNAME;  
    conn_opts.password = PASSWORD;  

    int rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Publishing message: %s\n", PAYLOAD);
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = (int)strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to publish, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Message published\n");

    MQTTClient_disconnect(client, TIMEOUT);
    MQTTClient_destroy(&client);

    return 0;
}
