import ssl
import sys
import mysql.connector

import paho.mqtt.client #librería del MQTT

def on_connect(client, userdata, flags, rc): #rc vefica que la conexión se haya establecido. 
	print('connected (%s)' % client._client_id) #Le digo al cliente que esta conectado
	client.subscribe(topic='FI/SALON/CO2/#', qos=2) #Suscribo al cliente, QOS:quality of service

def on_message(client, userdata, message): #Cuando se reciba un mensaje
	print('------------------------------')
	print('topic: %s' % message.topic)
	print('payload: %s' % message.payload)
	print('qos: %d' % message.qos)
	mydb = mysql.connector.connect(
	host="localhost",
	user="CO2",
	password="1289",
	database="CO2"
	)
	
	mycursor = mydb.cursor()

	sql = "INSERT INTO Mediciones (Valor,ID_Sensor) VALUES (%s, %s)"
	val = (int (message.payload), int (message.topic.split("/")[-1]))
	mycursor.execute(sql, val)

	mydb.commit()
	
	mycursor.close()
	mydb.close()
	

def main(): #Define un cliente
	client = paho.mqtt.client.Client(client_id='Karina-subs', clean_session=False) #Inventamos el id del cliente, clean session -> tipo de cliente. cuando esta en false el broker no va a olvidar la infor del cliente. 
	client.on_connect = on_connect
	client.on_message = on_message
	client.connect(host='127.0.0.1', port=1883)
	client.loop_forever()

if __name__ == '__main__':
	main()

sys.exit(0)
