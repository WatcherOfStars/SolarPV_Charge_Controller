from paho.mqtt import client as mqtt_client
import json
import time

broker = '192.168.4.1'
port = 9000
topic_pub = "pub_test"
topic_sub = "sub_test"
# generate client ID with pub prefix randomly
client_id = 'py_client'
username = 'solarpv'
password = 'solarpv123'
deviceId = "37"

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc==0:
            print("Successfully connected to MQTT broker")
        else:
            print("Failed to connect, return code %d", rc)


    client = mqtt_client.Client(client_id = client_id, callback_api_version = mqtt_client.CallbackAPIVersion.VERSION1)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def publish(client, status):
    msg = "{\"action\":\"command/insert\",\"deviceId\":\""+deviceId+"\",\"command\":{\"command\":\"LED_control\",\"parameters\":{\"led\":\""+status+"\"}}}"
    result = client.publish(topic_pub, msg)
    msg_status = result[0]
    if msg_status ==0:
        print(f"message : {msg} sent to topic {topic_pub}")
    else:
        print(f"Failed to send message to topic {topic_pub}")


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        #print(f"Recieved '{msg.payload.decode()}' from '{msg.topic}' topic")
        y = json.loads(msg.payload.decode())
        print("val:", y["val"])



    client.subscribe(topic_sub)
    client.on_message = on_message

def main():
    client = connect_mqtt()
    subscribe(client)


    pub_time = 5
    last_time = time.time()
    while True:
        client.loop()
        if(time.time() > last_time + pub_time):
            print("publishing")
            publish(client, "GOOD")
            last_time = time.time()
        

if __name__ == '__main__':
    main()