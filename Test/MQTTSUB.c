#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://210.246.215.31:1883"  // MQTT broker address
#define CLIENTID    "ExampleClientSub"      // Client ID
#define TOPIC       "test/topic"            // Topic to subscribe to
#define QOS         1                       // Quality of Service (0, 1, or 2)
#define TIMEOUT     10000L                  // Timeout in milliseconds

#define USERNAME    "tgr"         // Username for authentication
#define PASSWORD    "tgr18"         // Password for authentication

// Corrected callback function signature
int messageArrived(void* context, char* topicName, int topicLen, MQTTClient_message* message)
{
    printf("Message arrived: %s\n", (char*)message->payload);
    MQTTClient_freeMessage(&message);  // Free the message when done
    return 1;  // Return 1 to indicate message was successfully processed
}

int main(int argc, char* argv[])
{
    MQTTClient client;
    int rc;

    // Initialize MQTT client
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Set up the message handler with the corrected callback signature
    MQTTClient_setCallbacks(client, NULL, NULL, messageArrived, NULL);

    // Set connection options
    MQTTClient_connectOptions connectOptions = MQTTClient_connectOptions_initializer;
    connectOptions.MQTTVersion = MQTTVERSION_3_1_1;  // Correct MQTT version
    connectOptions.username = USERNAME;
    connectOptions.password = PASSWORD;
    connectOptions.keepAliveInterval = 20;
    connectOptions.cleansession = 1;

    // Connect to the broker
    if ((rc = MQTTClient_connect(client, &connectOptions)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    // Subscribe to the topic
    if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to subscribe, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribed to topic %s\n", TOPIC);

    // Wait for messages
    while (1)
    {
        MQTTClient_yield();  // Allow MQTT client to process messages
    }

    // Cleanup
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);

    return 0;
}
