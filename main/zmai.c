/* 
****************************************************************
	zmai main
	Lutov Andrey  Donetsk
Use for compilation ESP-IDF Programming Guide:
https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/
****************************************************************
*/
#define AP_VER "2021.04.20"
#include "zmai.h"



//**************** my common proc *******************
// bin byte to hex char string
void bin2hex(const unsigned char *bin, char *out, size_t len)
{
	size_t  i;
	if (bin == NULL || out == NULL || len == 0) return;
	for (i=0; i<len; i++) {
		out[i*2]   = "0123456789abcdef"[bin[i] >> 4];
		out[i*2+1] = "0123456789abcdef"[bin[i] & 0x0F];
	}
	out[len*2] = '\0';
}

// parse uri par=value string like this:
// swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
// smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2
// find par *key in string *cin(isize) & copy value to *out buffer(osize) 
void parsuri(char *cin, char *cout, char *ckey, int isize, int osize)
{
	int i = 0;
	int j = 0;
	char found = 0;
	char a;
	char b;
	char c;
	while ((i < isize) && (!found)) {
	a = cin[i];
	b = ckey[j];
	i++;
	j++;
	if (a != b) {
	if ((a == 0x3d) && (b == 0)) {                 //0x3d-> =
	found = 1;
	cout[0] = 0;
	}
	if (a < 0x20) i = isize;
	j = 0;
	}
	}	
	j = 0;
	while ((i < isize) && (j < osize) && (found)) {
	a = cin[i];
	if ((a > 0x1f) && (a != 0x26)) {       //0x26-> &
	if (a == 0x25) {
	i++;
	a = cin[i];
	b =  (a <= '9') ? (a - '0') : (a - 'A' + 10);
	b = (b & 0xf)*16;
	i++;
	a = cin[i];
	c =  (a <= '9') ? (a - '0') : (a - 'A' + 10);
	c = (c & 0xf) + b;
	cout[j] = c;
	} else cout[j] = cin[i];
	i++; j++;
	cout[j] = 0;
	} else found = 0;
	}
}

//return offset after founded string 0 if not found
int parsoff(char *cin, char *ckey, int isize)
{
	int i = 0;
	int j = 0;
	char found = 0;
	char a;
	char b;
	while ((i < isize) && (!found)) {
	a = cin[i];
	b = ckey[j];
	i++;
	j++;
	if (a != b) {
	if (b == 0) {
	found = 1;
	i--;
	}
	j = 0;
	}
	}	
	if (!found) i = 0;
	return i;
}

//my string copy include dest buf limit
void mystrcpy(char *cout, char *cin, int osize)
{
	int i = 0;
	char a;
	while (i < osize) {
	a = cin[i];
	if  (a > 0x1f) {
	cout[i] = a;
	i++;
	cout[i] = 0;
	} else {
	i = osize;
	}		
	}
}


// size control case insensitive strings compare 
bool incascmp(char *cain, char *cbin, int size)
{
	int i = 0;
	char ca;
	char cb;
	bool result = 0;
	while ((i < size) && (!result)) {
	ca = cain[i];
	cb = cbin[i];
	i++;
	if (ca >= 'A' && ca <= 'Z')
	ca = 'a' + (ca - 'A');
	if (cb >= 'A' && cb <= 'Z')
	cb = 'a' + (cb - 'A');
	if ((ca==0) || (ca != cb)) result = 1;
	}
	if (cain[i] != 0) result = 1;
	return result;
}

// size control case sensitive strings compare 
bool inccmp(char *cain, char *cbin, int size)
{
	int i = 0;
	char ca;
	char cb;
	bool result = 0;
	while ((i < size) && (!result)) {
	ca = cain[i];
	cb = cbin[i];
	i++;
	if ((ca==0) || (ca != cb)) result = 1;
	}
	if (cain[i] != 0) result = 1;
	return result;
}

// store uptime string in *cout buffer
void uptime_string_exp(char *cout)
{
	char buff[16];
	int minutes = tuptime / 7500;
	int days = minutes / 1440;
	minutes = minutes % 1440;
	int hrs = minutes / 60;
	minutes = minutes % 60;
       	itoa(days,buff,10);
	strcpy (cout,buff);
	strcat (cout," days ");
       	itoa(hrs,buff,10);
	strcat (cout,buff);
	strcat (cout," hours ");
       	itoa(minutes,buff,10);
	strcat (cout,buff);
	strcat (cout," minutes");
}

  /**
     Time functions
  */
void hw_timer_callback(void *arg)
{
	tuptime++;

	if (mcmsnd) mcmsnd--;
}

void MqttPubSub () {
	char buff[16];
	char buft[64];
	char bufd[2048];
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/status");
	esp_mqtt_client_publish(mqttclient, buft, "online", 0, 1, 1);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass && tESP8266Addr[0]) {
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tESP8266Addr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"ZMAi");
	if (zmainum)  {
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,".Rssi\",\"icon\":\"mdi:wifi\",\"uniq_id\":\"Rssi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"ZMAi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\"],\"name\":\"ZMAi-90");
	if (zmainum)  {
	strcat(bufd, ".");
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,"\",\"model\":\"ZMAi-90\",\"manufacturer\":\"Tuya Smart Home Automation\"},");
	strcat(bufd,"\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tESP8266Addr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"ZMAi");
	if (zmainum)  {
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,".Switch\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"Switch_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"ZMAi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\"],\"name\":\"ZMAi-90");
	if (zmainum)  {
	strcat(bufd, ".");
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,"\",\"model\":\"ZMAi-90\",\"manufacturer\":\"Tuya Smart Home Automation\"},");
	strcat(bufd,"\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tESP8266Addr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"ZMAi");
	if (zmainum)  {
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,".Voltage\",\"icon\":\"mdi:alpha-v-circle-outline\",\"uniq_id\":\"Voltage_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"ZMAi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\"],\"name\":\"ZMAi-90");
	if (zmainum)  {
	strcat(bufd, ".");
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,"\",\"model\":\"ZMAi-90\",\"manufacturer\":\"Tuya Smart Home Automation\"},");
	strcat(bufd,"\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/Voltage\",\"unit_of_meas\":\"V\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tESP8266Addr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"ZMAi");
	if (zmainum)  {
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,".Current\",\"icon\":\"mdi:alpha-a-circle-outline\",\"uniq_id\":\"Current_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"ZMAi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\"],\"name\":\"ZMAi-90");
	if (zmainum)  {
	strcat(bufd, ".");
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,"\",\"model\":\"ZMAi-90\",\"manufacturer\":\"Tuya Smart Home Automation\"},");
	strcat(bufd,"\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/Current\",\"unit_of_meas\":\"A\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/5x");
	strcat(buft,tESP8266Addr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"ZMAi");
	if (zmainum)  {
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,".Power\",\"icon\":\"mdi:flash-outline\",\"uniq_id\":\"Power_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"ZMAi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\"],\"name\":\"ZMAi-90");
	if (zmainum)  {
	strcat(bufd, ".");
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,"\",\"model\":\"ZMAi-90\",\"manufacturer\":\"Tuya Smart Home Automation\"},");
	strcat(bufd,"\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/Power\",\"unit_of_meas\":\"W\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/6x");
	strcat(buft,tESP8266Addr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"ZMAi");
	if (zmainum)  {
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,".Energy\",\"icon\":\"mdi:circle-slice-3\",\"uniq_id\":\"Energy_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"ZMAi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\"],\"name\":\"ZMAi-90");
	if (zmainum)  {
	strcat(bufd, ".");
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,"\",\"model\":\"ZMAi-90\",\"manufacturer\":\"Tuya Smart Home Automation\"},");
	strcat(bufd,"\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/Energy\",\"unit_of_meas\":\"kWh\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/7x");
	strcat(buft,tESP8266Addr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"ZMAi");
	if (zmainum)  {
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,".EnergyYesterday\",\"icon\":\"mdi:circle-slice-3\",\"uniq_id\":\"EnergyYesterday_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"ZMAi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\"],\"name\":\"ZMAi-90");
	if (zmainum)  {
	strcat(bufd, ".");
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,"\",\"model\":\"ZMAi-90\",\"manufacturer\":\"Tuya Smart Home Automation\"},");
	strcat(bufd,"\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/EnergyYesterday\",\"unit_of_meas\":\"kWh\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/8x");
	strcat(buft,tESP8266Addr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"ZMAi");
	if (zmainum)  {
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,".EnergyToday\",\"icon\":\"mdi:circle-slice-3\",\"uniq_id\":\"EnergyToday_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"ZMAi_");
	strcat(bufd,tESP8266Addr);
	strcat(bufd,"\"],\"name\":\"ZMAi-90");
	if (zmainum)  {
	strcat(bufd, ".");
	itoa(zmainum,buff,10);
	strcat(bufd, buff);
	}
	strcat(bufd,"\",\"model\":\"ZMAi-90\",\"manufacturer\":\"Tuya Smart Home Automation\"},");
	strcat(bufd,"\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/EnergyToday\",\"unit_of_meas\":\"kWh\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//


	}
}

//******************* Mqtt **********************
//static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
//    esp_mqtt_client_handle_t client = event->client;
    // your_context_t *context = event->context;
	int topoff = 0;
	char tbuff[128];
	char ttopic[128];
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
//	mqttConnected = false;
//	ESP_LOGI(TAG,"MQTT_EVENT_CONNECTED");
	mcmsnd = 375;
	bPState = 255;
	bPCount = 0;
	bPCount--;
	bPCountT = bPCount;
	bPCountY = bPCount;
	bPCurrent = bPCount;
	bPVoltage = bPCount;
	bPPower = bPCount;
	iprevRssiESP = 0;
	MqttPubSub ();
	mqttConnected = true;
	break;

	case MQTT_EVENT_DISCONNECTED:
	mcmsnd = 375;
	bPCount = 0;
	bPCount--;
	bPCurrent = bPCount;
	bPVoltage = bPCount;
	bPPower = bPCount;
	iprevRssiESP = 0;
	mqttConnected = false;
//	ESP_LOGI(TAG,"MQTT_EVENT_DISCONNECTED");
	break;

        case MQTT_EVENT_SUBSCRIBED:
//            ESP_LOGI(TAG,"MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
	break;
	case MQTT_EVENT_UNSUBSCRIBED:
//            ESP_LOGI(TAG,"MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
	break;
	case MQTT_EVENT_PUBLISHED:
//            ESP_LOGI(TAG,"MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
	break;

	case MQTT_EVENT_DATA:
        memcpy(ttopic, event->topic, event->topic_len);
	ttopic[event->topic_len] = 0;
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	topoff = parsoff(event->topic,tbuff, event->topic_len);
	if (topoff) {
	if (!memcmp(event->topic+topoff, "state", event->topic_len-topoff)) {
        if (!mcmsnd) bPState = 255;
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
        if (!mcmsnd || !bState) {
	switch (sChip) {
	case 1:
	ttopic[0] = 0x55;
	ttopic[1] = 0xaa;
	ttopic[2] = 0x00;
	ttopic[3] = 0x06;
	ttopic[4] = 0x00;
	ttopic[5] = 0x05;
	ttopic[6] = 0x01;
	ttopic[7] = 0x01;
	ttopic[8] = 0x00;
	ttopic[9] = 0x01;
	ttopic[10] = 0x01;
	ttopic[11] = 0x0e;
        uart_write_bytes(UART_NUM_0, ttopic, 12);
	break;
	case 2:
	ttopic[0] = 0x55;
	ttopic[1] = 0xaa;
	ttopic[2] = 0x00;
	ttopic[3] = 0x06;
	ttopic[4] = 0x00;
	ttopic[5] = 0x05;
	ttopic[6] = 0x10;
	ttopic[7] = 0x01;
	ttopic[8] = 0x00;
	ttopic[9] = 0x01;
	ttopic[10] = 0x01;
	ttopic[11] = 0x1d;
        uart_write_bytes(UART_NUM_0, ttopic, 12);
	break;
	}
        }
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
        if (!mcmsnd || bState) {
	switch (sChip) {
	case 1:
	ttopic[0] = 0x55;
	ttopic[1] = 0xaa;
	ttopic[2] = 0x00;
	ttopic[3] = 0x06;
	ttopic[4] = 0x00;
	ttopic[5] = 0x05;
	ttopic[6] = 0x01;
	ttopic[7] = 0x01;
	ttopic[8] = 0x00;
	ttopic[9] = 0x01;
	ttopic[10] = 0x00;
	ttopic[11] = 0x0d;
        uart_write_bytes(UART_NUM_0, ttopic, 12);
	break;
	case 2:
	ttopic[0] = 0x55;
	ttopic[1] = 0xaa;
	ttopic[2] = 0x00;
	ttopic[3] = 0x06;
	ttopic[4] = 0x00;
	ttopic[5] = 0x05;
	ttopic[6] = 0x10;
	ttopic[7] = 0x01;
	ttopic[8] = 0x00;
	ttopic[9] = 0x01;
	ttopic[10] = 0x00;
	ttopic[11] = 0x1c;
        uart_write_bytes(UART_NUM_0, ttopic, 12);
	break;
	}

        }
	}

	}
	}
	break;




	case MQTT_EVENT_ERROR:
//	ESP_LOGI(TAG,"MQTT_EVENT_ERROR");
	break;
	default:
//	ESP_LOGI(TAG,"Other event id:%d", event->event_id);
	break;
	}
    return ESP_OK;
}


/*
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
//    ESP_LOGI(TAG,"Event dispatched from event loop base=%s, event_id=%d\n", base, event_id);
    mqtt_event_handler_cb(event_data);
}
*/
static void mqtt_app_start(void)
{
	char buff[16];
	char luri[128];
	char llwtt[16];
	strcpy(luri,"mqtt://");
	strcat(luri,MQTT_USER);
	strcat(luri,":");
	strcat(luri,MQTT_PASSWORD);
	strcat(luri,"@");
	strcat(luri,MQTT_SERVER);
	strcat(luri,":");
	itoa(mqtt_port,buff,10);
	strcat(luri,buff);
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/status");

	esp_mqtt_client_config_t mqtt_cfg = {
	.uri = luri,
	.lwt_topic = llwtt,
	.lwt_msg = "offline",
	.keepalive = 60,
	.client_id = MQTT_CLIENT_NAME,
	.buffer_size = 2048,
        .event_handle = mqtt_event_handler,
	};
	mqttConnected =false;
	mqttclient = esp_mqtt_client_init(&mqtt_cfg);
//	esp_mqtt_client_register_event(mqttclient, ESP_EVENT_ANY_ID, mqtt_event_handler, mqttclient);
	esp_mqtt_client_start(mqttclient);
}

//******************* WiFi **********************
static EventGroupHandle_t wifi_event_group;

static int s_retry_num = 0;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;
    switch (event->event_id) {
	case SYSTEM_EVENT_STA_START:
	tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA,MQTT_CLIENT_NAME);
		esp_wifi_connect();
		break;


	case SYSTEM_EVENT_STA_GOT_IP:
		xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
		s_retry_num = 0;

            break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
//            ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);
            if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
                /*Switch to 802.11 bgn mode */
                esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
            }
	tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA,MQTT_CLIENT_NAME);
	if (s_retry_num < WIFI_MAXIMUM_RETRY) {
		esp_wifi_connect();
		s_retry_num++;
		xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
	} else {
	xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
	}
		break;
	default:
		break;
    }
    return ESP_OK;
}





void wifi_init_sta(void)
{
	s_retry_num = 0;
	tcpip_adapter_init();
	wifi_event_group = xEventGroupCreate();
	esp_event_loop_init(wifi_event_handler, NULL);
//	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));


	wifi_config_t wifi_config = {
        .sta = {
		.threshold.authmode = WIFI_AUTH_WPA_PSK
        },
    };

	memcpy(wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
	memcpy(wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N));
	ESP_ERROR_CHECK(esp_wifi_start() );
	EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, false, false, portMAX_DELAY);
	if (bits & WIFI_FAIL_BIT) {
	ESP_ERROR_CHECK(esp_wifi_stop() );
	char DEFWFSSID[33];
	char DEFWFPSW[65];
	strcpy(DEFWFSSID,"zmai");
	strcpy(DEFWFPSW,"12345678");
	memcpy(wifi_config.sta.ssid, DEFWFSSID, sizeof(wifi_config.sta.ssid));
	memcpy(wifi_config.sta.password, DEFWFPSW, sizeof(wifi_config.sta.password));
	bits = xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );
	bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, false, false, portMAX_DELAY);
	if (bits & WIFI_FAIL_BIT) {
	esp_restart();
	}
	}
}


//*************** http server *******************

/* HTTP GET main handler */
static esp_err_t pmain_get_handler(httpd_req_t *req)
{
	int FreeMem = esp_get_free_heap_size();
	char bufip[32] = {0};
	time_t now;
	char strftime_buf[64];
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
//ESP_LOGI(TAG,"Ip from header %s", bufip); 
        }
    }
	char bsend[14000];
        char buff[64];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>ZMAi</title>");
        strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ZMAi-90");
	if (zmainum)  {
	strcat(bsend, ".");
	itoa(zmainum,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend,"</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu active' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='resetesp'>&#128259;<span class='showmenulabel'>Reset ESP");
	strcat(bsend,"</span></a><a class='menu' href='loadesp'>&#10548;<span class='showmenulabel'>Load ESP Firmware");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a></div></header>");
      // Live data
	strcat(bsend,"<h3>ZMAi-90 Live Data *</h3>");
	strcat(bsend,"<input type=\"button\" value =\"Pause\" onclick=\"stopUpdate();\" />");                                                            // Pause
	strcat(bsend,"<input type=\"button\" value =\"Restart\" onclick=\"restartUpdate();\" />");                                                       // Restart
	strcat(bsend,"<input type=\"button\" value =\"Refresh\" onclick=\"window.location.reload(true);\" />");                                          // Refresh
	strcat(bsend,"<input type=\"text\" id=\"mySearch\" onkeyup=\"filterLines()\" placeholder=\"Search for...\" title=\"Type in a name\"><br />");    // Search
	strcat(bsend,"<table id=\"liveData\" class='multirow';>");                                                                                       // Table of x lines
	strcat(bsend,"<tr class=\"header\"><th style='text-align:left;'>Raw Data</th><th style='text-align:left;'> MQTT Topic </th><th style='text-align:left;'> MQTT Data </th></tr>");

	for (int i = 0; i < (7); i++) { // not too high to avoid overflow
	strcat(bsend,"<tr id=\"data");
	itoa(i,buff,10);
	strcat(bsend, buff);
	strcat(bsend,"\"><td></td><td></td><td></td></tr>");
	}
        strcat(bsend,"</table>");
        strcat(bsend,"<script>function filterLines() {");  // Script to filter lines
        strcat(bsend," var input, filter, table, tr, td, i;");
        strcat(bsend," input = document.getElementById(\"mySearch\");");
        strcat(bsend," filter = input.value.toUpperCase();");
        strcat(bsend," table = document.getElementById(\"liveData\");");
        strcat(bsend," tr = table.getElementsByTagName(\"tr\");");
        strcat(bsend," for (i = 0; i < tr.length; i++) {");
        strcat(bsend,"  td = tr[i].getElementsByTagName(\"td\")[0];");
        strcat(bsend,"  if (td) { if (td.innerHTML.toUpperCase().indexOf(filter) > -1) {");
        strcat(bsend,"  tr[i].style.display = \"\"; } else {  tr[i].style.display = \"none\";"); 
        strcat(bsend," }}}}</script><script>"); // Script to update data and move to next line
        strcat(bsend,"var x = setInterval(function() {loadData(\"data.txt\",updateData)}, 200);");
        strcat(bsend,"function loadData(url, callback){ var xhttp = new XMLHttpRequest();");
        strcat(bsend,"xhttp.onreadystatechange = function(){ if(this.readyState == 4 && this.status == 200){");
        strcat(bsend," callback.apply(xhttp);}};xhttp.open(\"GET\", url, true);xhttp.send();}");
        strcat(bsend,"var memorized_data; function updateData(){ if (memorized_data != this.responseText) {");
	for (int i = (7 - 1); i > 0; i--) {	// not too high to avoid overflow
        strcat(bsend,"document.getElementById(\"data");
	itoa(i,buff,10);
	strcat(bsend, buff);
        strcat(bsend,"\").innerHTML = document.getElementById(\"data");
	i--;
	itoa(i,buff,10);
	strcat(bsend, buff);
	i++;
        strcat(bsend,"\").innerHTML;");
	}
        strcat(bsend,"} document.getElementById(\"data0\").innerHTML = this.responseText;");
        strcat(bsend," memorized_data = this.responseText;"); // memorize new data
        strcat(bsend,"filterLines();}"); // apply filter from mySearch input
        strcat(bsend,"function stopUpdate(){ clearInterval(x);} function restartUpdate(){");
        strcat(bsend," x = setInterval(function() {loadData(\"data.txt\",updateData)}, 200);}</script>");
        strcat(bsend,"<div class='note'>* see Live \"Data tab\" for more lines - web view may not catch all frames, MQTT debug is more accurate</div>");
		// System Info
        strcat(bsend,"<h3>System Info</h3><table class='normal'><tr><td style='min-width:150px;'>Version</td><td style='width:80%;'>");
        strcat(bsend,AP_VER);
/*
	strcat(bsend,"</td></tr><tr><td>ESP-IDF version</td><td>");
	strcat(bsend,IDF_VER);
*/
	strcat(bsend,"</td></tr><tr><td>Local time and date</td><td>");
	strcat(bsend,strftime_buf);
        strcat(bsend,"</td></tr><tr><td>Uptime</td><td>");
	uptime_string_exp(buff);
	strcat(bsend,buff);
        strcat(bsend,"</td></tr><tr><td>Free memory</td><td>");
	itoa(FreeMem,buff,10);
	strcat(bsend,buff);
        strcat(bsend," bytes</td></tr>");
	wifi_ap_record_t wifidata;
	if (esp_wifi_sta_get_ap_info(&wifidata)==0){
        strcat(bsend,"<tr><td>WiFi network</td><td>");
        buff[31] = 0;
	memcpy(buff,wifidata.ssid,31);
        strcat(bsend,buff);
	strcat(bsend,"</td></tr><tr><td>WiFi RSSI</td><td>");
        itoa(wifidata.rssi,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB</td></tr><tr><td>WiFi IP address</td><td>");
        strcat(bsend,bufip);
	strcat(bsend,"</td></tr>");
	}
	strcat(bsend,"<tr><td>MQTT server:port/state</td><td>");
        if (MQTT_SERVER[0]) strcat(bsend,MQTT_SERVER);
	strcat(bsend,":");
	itoa(mqtt_port,buff,10);
	strcat(bsend,buff);
        (mqttConnected)? strcat(bsend,"/Connected") : strcat(bsend,"/Disconnected");

	strcat(bsend,"</td></tr></table><br><span style=\"font-size: 0.9em\">Running for ");
	uptime_string_exp(buff);
	strcat(bsend,buff);
        strcat(bsend," </span><input type=\"button\" value =\"Refresh\" onclick=\"window.location.reload(true);\" />");
	strcat(bsend,"<br><footer><h6>More info on <a href='https://github.com/alutov/zmai-90_mqtt_gateway' style='font-size: 15px; text-decoration: none'>github.com/alutov </a></h6>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);
/*
	strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5;URL=http://");
	strcat(bsend,bufip);
	strcat(bsend,"/\"</body></html>");
*/
	httpd_resp_send(req, bsend, strlen(bsend));



    return ESP_OK;
}

static const httpd_uri_t pmain = {
	.uri       = "/",
	.method    = HTTP_GET,
	.handler   = pmain_get_handler,
	.user_ctx  = NULL
};




/* HTTP GET data.txt handler */
static esp_err_t pdatatxt_get_handler(httpd_req_t *req)
{
	char bsend[512];
	bsend[0] = 0;	
	strcpy(bsend,"<td>");
	strcat(bsend,bufferlnhex);
	strcat(bsend,"</td><td>");
	strcat(bsend,MQTT_TOPIC);
	strcat(bsend,"</td><td>");
	strcat(bsend,MQTT_DATA);
	strcat(bsend,"</td>");
	httpd_resp_send(req, bsend, strlen(bsend));
    return ESP_OK;
}
static const httpd_uri_t pdatatxt = {
	.uri       = "/data.txt",
	.method    = HTTP_GET,
	.handler   = pdatatxt_get_handler,
	.user_ctx  = NULL
};


/* HTTP GET reboot esp handler */
static esp_err_t presetesp_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
//save ip from header
	char bufip[32] = {0};
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
        }
	}
	strcpy(buf1,"<!DOCTYPE html><html><head><title>ZMAi</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>Rebooting esp...</body></html><meta http-equiv=\"refresh\" content=\"3;URL=http://");
        strcat(buf1, bufip);
	strcat(buf1,"/\"</body></html>");
	httpd_resp_send(req, buf1, strlen(buf1));

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	esp_restart();
    return ESP_OK;
}
static const httpd_uri_t presetesp = {
	.uri       = "/resetesp",
	.method    = HTTP_GET,
	.handler   = presetesp_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET setting handler */
static esp_err_t psetting_get_handler(httpd_req_t *req)
{
	char bsend[10000];
        char buff[32];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>ZMAi</title>");
        strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ZMAi-90");
	if (zmainum)  {
	strcat(bsend, ".");
	itoa(zmainum,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend,"</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='resetesp'>&#128259;<span class='showmenulabel'>Reset ESP");
	strcat(bsend,"</span></a><a class='menu' href='loadesp'>&#10548;<span class='showmenulabel'>Load ESP Firmware");
	strcat(bsend,"</span></a><a class='menu active' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a></div>");
	strcat(bsend,"</header><table class='normal'><h3>Wifi Setting</h3><br/><body>");
	strcat(bsend,"<form method=\"POST\" action=\"/setsave\"><input name=\"swfid\" value=\"");
	if (WIFI_SSID[0]) strcat(bsend,WIFI_SSID);
	strcat(bsend,"\" size=\"15\">SSID &emsp;<input type=\"password\" input name=\"swfpsw\" value=\"");
	if (WIFI_PASSWORD[0]) strcat(bsend,WIFI_PASSWORD);
	strcat(bsend,"\"size=\"31\">Password</br><h3>MQTT Setting</h3><br/><input name=\"smqsrv\" value=\"");
	if (MQTT_SERVER[0]) strcat(bsend,MQTT_SERVER);
	strcat(bsend,"\"size=\"15\">Server &emsp;<input name=\"smqprt\" type=\"number\" value=\"");
	itoa(mqtt_port,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"65535\" size=\"5\">Port &emsp;<input name=\"smqid\" value=\"");
	if (MQTT_USER[0]) strcat(bsend,MQTT_USER);
	strcat(bsend,"\"size=\"15\">Login &emsp;<input type=\"password\" input name=\"smqpsw\" value=\"");
	if (MQTT_PASSWORD[0]) strcat(bsend,MQTT_PASSWORD);
	strcat(bsend,"\"size=\"15\">Password</br><input type=\"checkbox\" name=\"chk1\" value=\"1\"");
	if (FDHass) strcat(bsend,"checked");
	strcat(bsend,"> Hass Discovery&emsp;<input type=\"checkbox\" name=\"chk2\" value=\"2\"");
	if (ftrufal) strcat(bsend,"checked");
        strcat(bsend,"> \"true/false\" Response(\"ON/OFF\" only if Hass Discovery)</br>");
	strcat(bsend,"<h3>System Setting</h3></br><input name=\"sespnum\" type=\"number\" value=\"");
	itoa(zmainum,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"9\" size=\"2\">ZMAi-90 Number &emsp;<input name=\"timzon\" type=\"number\" value=\"");
	uint8_t TmZn = TimeZone;
	if (TmZn >127) {
	TmZn = ~TmZn;
	TmZn++;
	strcat(bsend,"-");
	}
	itoa(TmZn,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"-12\" max=\"12\" size=\"3\">GMT Timezone</br>");
	strcat(bsend,"<select name=\"schip\"><option ");
	if (sChip == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">V9821</option><option ");
	if (sChip == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">V9821S</option></select>Metering chip</br>");
	strcat(bsend,"Energy offset: <input type=\"checkbox\" name=\"chk3\" value=\"3\"");
	if (bCountSign) strcat(bsend,"checked");
	strcat(bsend,"> - <input name=\"scoffs\" type=\"number\" value=\"");
	itoa(bCountOffs,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"4294967295\" size=\"10\">Wh </br>");
	strcat(bsend,"<input type=\"checkbox\" name=\"chk4\" value=\"4\"");
	strcat(bsend,"> Format NVS area</br>");
	strcat(bsend,"<h3>Store setting then press SAVE. ZMAi-90 will restart</h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Save settings\"></form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

	httpd_resp_send(req, bsend, strlen(bsend));
    return ESP_OK;
}

static const httpd_uri_t psetting = {
	.uri       = "/setting",
	.method    = HTTP_GET,
	.handler   = psetting_get_handler,
	.user_ctx  = NULL
};


static esp_err_t psetsave_get_handler(httpd_req_t *req)
{
	char buf1[1024] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
//save ip from header
	char bufip[32] = {0};
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
        }
	}
	int  ret;
	ret = httpd_req_recv(req,buf1,1024);
	if ( ret > 0 ) {
//ESP_LOGI(TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	strcpy(buf2,"swfid");
	parsuri(buf1,WIFI_SSID,buf2,1024,33);
	strcpy(buf2,"swfpsw");
	parsuri(buf1,WIFI_PASSWORD,buf2,1024,65);
	strcpy(buf2,"smqsrv");
	parsuri(buf1,MQTT_SERVER,buf2,1024,16);
	strcpy(buf2,"smqid");
	parsuri(buf1,MQTT_USER,buf2,1024,16);
	strcpy(buf2,"smqpsw");
	parsuri(buf1,MQTT_PASSWORD,buf2,1024,16);
	strcpy(buf2,"smqprt");
	parsuri(buf1,buf3,buf2,1024,8);
	mqtt_port = atoi(buf3);
	strcpy(buf2,"sespnum");
	parsuri(buf1,buf3,buf2,1024,4);
	zmainum = atoi(buf3);
	strcpy(buf2,"timzon");
	parsuri(buf1,buf3,buf2,1024,4);
	TimeZone = atoi(buf3);
	strcpy(buf2,"scoffs");
	parsuri(buf1,buf3,buf2,1024,11);
	bCountOffs = atoi(buf3);
	strcpy(buf2,"schip");
	parsuri(buf1,buf3,buf2,1024,4);
	sChip = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"chk1");
	parsuri(buf1,buf3,buf2,1024,2);
	FDHass = 0;
	if (buf3[0] == 0x31) FDHass = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk2");
	parsuri(buf1,buf3,buf2,1024,2);
	ftrufal = 0;
	if (buf3[0] == 0x32) ftrufal = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk3");
	parsuri(buf1,buf3,buf2,1024,2);
	bCountSign = 0;
	if (buf3[0] == 0x33) bCountSign = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk4");
	parsuri(buf1,buf3,buf2,1024,2);
	if (buf3[0] == 0x34) {
	nvs_flash_erase();
	nvs_flash_init();
	}


// write nvs
	nvs_handle my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {

	nvs_set_u32(my_handle, "scoffs", bCountOffs);
	nvs_set_u16(my_handle, "smqprt", mqtt_port);
	nvs_set_u8(my_handle,  "sespnum", zmainum);
	nvs_set_u8(my_handle,  "timzon", TimeZone);
	nvs_set_u8(my_handle,  "schip", sChip);
	nvs_set_u8(my_handle,  "chk1",  FDHass);
	nvs_set_u8(my_handle,  "chk2",  ftrufal);
	nvs_set_u8(my_handle,  "chk3",  bCountSign);

	nvs_set_str(my_handle, "swfid", WIFI_SSID);
	nvs_set_str(my_handle, "swfpsw", WIFI_PASSWORD);
	nvs_set_str(my_handle, "smqsrv", MQTT_SERVER);
	nvs_set_str(my_handle, "smqid", MQTT_USER);
	nvs_set_str(my_handle, "smqpsw", MQTT_PASSWORD);

        ret = nvs_commit(my_handle);
	if (ret != ESP_OK) {
//	ESP_LOGE(TAG, "NVS write error");
}
// Close nvs
	nvs_close(my_handle);
	}
	}
	strcpy(buf1,"<!DOCTYPE html><html><head><title>ZMAi</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>Setting saved. Rebooting...</body></html><meta http-equiv=\"refresh\" content=\"3;URL=http://");
        strcat(buf1, bufip);
	strcat(buf1,"/\"</body></html>");
	httpd_resp_send(req, buf1, strlen(buf1));

//	ESP_LOGI(TAG, "Prepare to restart system!");
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	esp_restart();
    return ESP_OK;
}
static const httpd_uri_t psetsave = {
	.uri       = "/setsave",
	.method    = HTTP_POST,
	.handler   = psetsave_get_handler,
	.user_ctx  = NULL
};

static esp_err_t psetignore_get_handler(httpd_req_t *req)
{
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}


static const httpd_uri_t psetignore = {
	.uri       = "/setignore",
	.method    = HTTP_POST,
	.handler   = psetignore_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET update esp handler */
static esp_err_t ploadesp_get_handler(httpd_req_t *req)
{
	httpd_resp_send(req, espserverIndex, strlen(espserverIndex));
    return ESP_OK;
}
static const httpd_uri_t ploadesp = {
	.uri       = "/loadesp",
	.method    = HTTP_GET,
	.handler   = ploadesp_get_handler,
	.user_ctx  = NULL
};


/* HTTP GET updating esp handler */
static esp_err_t ploadingesp_get_handler(httpd_req_t *req)
{
//	f_update = true;
	char otabuf[otabufsize] ={0};
	char filnam[128] ={0};
        int data_read = 0;
	int  otabufoffs = 0;
	esp_err_t err = 0;
	int binary_file_length = 0;
	bool image_header_was_checked = false;
	bool ota_running = true;
	esp_ota_handle_t update_handle = 0 ;
//save ip from header
	char bufip[32] = {0};
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
        }
	}
//	ESP_LOGI(TAG, "Starting OTA");
	const esp_partition_t *pupdate = esp_ota_get_next_update_partition(NULL);
	/*deal with all receive packet*/
// read loop
	while (ota_running) {
	otabufoffs = 0;
//read max 2048 bytes
        data_read = httpd_req_recv(req, otabuf, otabufsize);
        if (data_read < 0) {
//	ESP_LOGE(TAG, "Error: data read error");
	ota_running = false;
	otabufoffs = 0;
        } else if (data_read > 0) {
	if (image_header_was_checked == false) {
// if first read check and remove POST header
/*
in otabuf after httpd_req_recv string like below

------WebKitFormBoundary2ZdUwM7CAu6TtDPq
Content-Disposition: form-data; name="update"; filename="zmai.bin"
Content-Type: application/octet-stream\r\n\r\n
*/

//check "Content-Disposition" string in received data
	otabufoffs = parsoff(otabuf,"Content-Disposition", otabufsize);
	if (!otabufoffs) {
//	ESP_LOGE(TAG, "Content-Disposition not found");
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}

// save filename
	otabufoffs = parsoff(otabuf,"filename=", otabufsize);
	mystrcpy(filnam, otabuf+otabufoffs, 127);
// search for data begin
	otabufoffs = parsoff(otabuf,"application/octet-stream\r\n\r\n", otabufsize);
	if (!otabufoffs) {
//	ESP_LOGE(TAG, "application/octet-stream not found");
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}

//esp_log_buffer_hex(TAG, otabuf, 128);
//	ESP_LOGI(TAG, "Loading filename: %s",filnam);

	image_header_was_checked = true;
	err = esp_ota_begin(pupdate, OTA_SIZE_UNKNOWN, &update_handle);
	if (err != ESP_OK) {
//	ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}
//	ESP_LOGI(TAG, "esp_ota_begin succeeded");
	} 
	if (data_read > 0)  {
	err = esp_ota_write(update_handle, (const void *)otabuf+otabufoffs, data_read-otabufoffs);
	if (err != ESP_OK) {
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}

	}
	binary_file_length = binary_file_length  + data_read - otabufoffs; 
//ESP_LOGI(TAG, "Written image length %x, offset %x", binary_file_length, otabufoffs);
	} else if (data_read == 0) {
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}
}
//
//	ESP_LOGI(TAG, "Total Write binary data length: %x", binary_file_length);
	
	err = esp_ota_end(update_handle);
	strcpy (otabuf,"<!DOCTYPE html><html><head><title>ZMAi</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>Update ");
	if (err != ESP_OK) {
	if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
//	ESP_LOGE(TAG, "Image validation failed, image is corrupted");
	}
//	ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
	strcat(otabuf,"failed");
	} else {
//	ESP_LOGI(TAG, "esp_ota_end ok!");
	strcat(otabuf,"ok");
	}
	strcat(otabuf,". Rebooting...</body></html><meta http-equiv=\"refresh\" content=\"3;URL=http://");
        strcat(otabuf, bufip);
	strcat(otabuf,"/\"</body></html>");
	httpd_resp_send(req, otabuf, strlen(otabuf));

	err = esp_ota_set_boot_partition(pupdate);
	if (err != ESP_OK) {
//	ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
	ota_running = false;
	}
//	ESP_LOGI(TAG, "Prepare to restart system!");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	esp_restart();
    return ESP_OK;
}
static const httpd_uri_t ploadingesp = {
	.uri       = "/loadingesp",
	.method    = HTTP_POST,
	.handler   = ploadingesp_get_handler,
	.user_ctx  = NULL
};





//*************************************************
httpd_handle_t start_webserver(void)
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_uri_handlers = 16;
//	config.max_resp_headers =16;
	config.stack_size = 20480;

    // Start the httpd server
/*
	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	ESP_LOGI(TAG, "Max URI handlers: '%d'", config.max_uri_handlers);
	ESP_LOGI(TAG, "Max Open Sessions: '%d'", config.max_open_sockets);
	ESP_LOGI(TAG, "Max Header Length: '%d'", HTTPD_MAX_REQ_HDR_LEN);
	ESP_LOGI(TAG, "Max URI Length: '%d'", HTTPD_MAX_URI_LEN);
	ESP_LOGI(TAG, "Max Stack Size: '%d'", config.stack_size);
*/
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
//ESP_LOGI(TAG, "Registering URI handlers");
	httpd_register_uri_handler(server, &pmain);
	httpd_register_uri_handler(server, &pdatatxt);
	httpd_register_uri_handler(server, &presetesp);
	httpd_register_uri_handler(server, &psetting);
	httpd_register_uri_handler(server, &psetsave);
	httpd_register_uri_handler(server, &psetignore);
	httpd_register_uri_handler(server, &ploadesp);
	httpd_register_uri_handler(server, &ploadingesp);

        return server;
    }

//    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static httpd_handle_t server = NULL;




//******************* uart **********************
bool rxlnread()
{
	bool done = false;
	bool ready = false;
	uint8_t a;
	uint8_t sz;
	uint8_t sum;
	while (!done) {
	if (cptbufrx >= lenbufrx) {
	uint rx_size;	
	uart_get_buffered_data_len(UART_NUM_0, &rx_size);
	if (rx_size) {
        lenbufrx = uart_read_bytes(UART_NUM_0, bufferrx, BUFFER_SIZE, 0 / portTICK_RATE_MS);
	cptbufrx = 0;
	if (lenbufrx < 0) lenbufrx = 0;
	if (!lenbufrx) done = true;
	} else done = true;
	}
	if (!done) {
	if (cptbufln >= (BUFFER_SIZE-1)) cptbufln = 0;
	a = bufferrx[cptbufrx];
	bufferln[cptbufln] = a;
	cptbufrx++;	
	cptbufln++;
	bufferln[cptbufln] = 0;
	if ((cptbufln == 1) && (a != 0x55)) cptbufln = 0;
	if ((cptbufln == 2) && (a != 0xaa)) cptbufln = 0;
	if ((cptbufln == 3) && (a != 0x03)) cptbufln = 0;
        if (cptbufln > 6) {
	sz = bufferln[5] + 6;
	if ((cptbufln == (sz + 1)) && (sz < 64)) {
	sum = 0;
	for (a = 0; a < sz; a++) sum = sum + bufferln[a];
	sum = sum ^ bufferln[sz];
	cptbufln = 0;
	done = true;
	if (!sum) {
	ready = true;
	bin2hex(bufferln, bufferlnhex, sz);
	}
	} else if ((cptbufln > (sz + 1)) || (sz > 63)) cptbufln = 0;
	if ((cptbufln == 0) && !ready) bufferln[0] = 0;
	}


	}
	}
	return ready;
}

// read case 1 
// 55 aa 03 04 00 00 - button
// 55 aa 03 07 00 05 01 01 00 01 01/00 - state
// 55 aa 03 07 00 08 11 02 00 04 00 00 00 00 - kWh
// 55 aa 03 07 00 08 12 02 00 04 00 00 00 00 - I
// 55 aa 03 07 00 08 13 02 00 04 00 00 00 00 - P
// 55 aa 03 07 00 08 14 02 00 04 00 00 08 d6 - U

// read case 2 
// 55 aa 03 00 00 01 01 - button ???
// 55 aa 03 07 00 05 10 01 00 01 00/01 - state ???
// 55 aa 03 07 00 08 01 02 00 04 00 00 00 00 - kWh

// 55 aa 03 07 00 08 0d 02 00 04 00 00 00 00 - ?

void rxlnparse()
{
	uint32_t tdata;
	char tbuff[16];
	MQTT_DATA[0] = 0;
	MQTT_TOPIC[0] = 0;
	if ((bufferln[0] != 0x55) || (bufferln[1] != 0xaa) || (bufferln[2] != 0x03)) return;
	switch (sChip) {
	case 1:
	if ((bufferln[3] == 0x07) && (bufferln[5] == 0x08) && (bufferln[7] == 0x02) && (bufferln[9] == 0x04)) {
	tdata = (bufferln[10] << 24) + (bufferln[11] << 16) + (bufferln[12] << 8) + bufferln[13];
	switch (bufferln[6]) { 
	case 0x11:
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/Energy");
	bCount = tdata;
	bCountR = tdata;
	if(!bCountSign) bCount = bCount + bCountOffs;
	else bCount = bCount - bCountOffs;
	tdata = bCount / 1000;
	itoa(tdata,tbuff,10);
	strcpy(MQTT_DATA,tbuff);
	strcat(MQTT_DATA,".");
	tdata = bCount % 1000;
	tdata = tdata / 10;
	if (tdata < 10) strcat(MQTT_DATA,"0");
	itoa(tdata,tbuff,10);
	strcat(MQTT_DATA,tbuff);
	if ((bCount != bPCount) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
        bPCount = bCount;
	}
	break;
	case 0x12:
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/Current");
	bCurrent = tdata;
	tdata = bCurrent / 1000;
	itoa(tdata,tbuff,10);
	strcpy(MQTT_DATA,tbuff);
	strcat(MQTT_DATA,".");
	tdata = bCurrent % 1000;
	if (tdata < 10) strcat(MQTT_DATA,"0");
	if (tdata < 100) strcat(MQTT_DATA,"0");
	itoa(tdata,tbuff,10);
	strcat(MQTT_DATA,tbuff);
	if ((bCurrent != bPCurrent) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
        bPCurrent = bCurrent;
	}
	break;
	case 0x13:
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/Power");
	bPower = tdata;
	tdata = bPower / 10;
	itoa(tdata,tbuff,10);
	strcpy(MQTT_DATA,tbuff);
	strcat(MQTT_DATA,".");
	tdata = bPower % 10;
	itoa(tdata,tbuff,10);
	strcat(MQTT_DATA,tbuff);
	if ((bPower != bPPower) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
        bPPower = bPower;
	}
	break;
	case 0x14:
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/Voltage");
	bVoltage = tdata;
	tdata = bVoltage / 10;
	itoa(tdata,tbuff,10);
	strcpy(MQTT_DATA,tbuff);
	strcat(MQTT_DATA,".");
	tdata = bVoltage % 10;
	itoa(tdata,tbuff,10);
	strcat(MQTT_DATA,tbuff);
	if ((bVoltage != bPVoltage) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
        bPVoltage = bVoltage;
	}
	break;
	}	
	} else if ((bufferln[3] == 0x07) && (bufferln[5] == 0x05) && (bufferln[6] == 0x01)) {
        bState = (bufferln[10]);
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/state");
	if (!bState) strcpy(MQTT_DATA,strOFF);
	else strcpy(MQTT_DATA,strON);
	if ((bState != bPState) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
	mcmsnd = 375;
        bPState = bState;
	}
	} else if ((bufferln[3] == 0x04) && (bufferln[5] == 0x00)) {
	if (!bState) {
	tbuff[0] = 0x55;
	tbuff[1] = 0xaa;
	tbuff[2] = 0x00;
	tbuff[3] = 0x06;
	tbuff[4] = 0x00;
	tbuff[5] = 0x05;
	tbuff[6] = 0x01;
	tbuff[7] = 0x01;
	tbuff[8] = 0x00;
	tbuff[9] = 0x01;
	tbuff[10] = 0x01;
	tbuff[11] = 0x0e;
	} else {
	tbuff[0] = 0x55;
	tbuff[1] = 0xaa;
	tbuff[2] = 0x00;
	tbuff[3] = 0x06;
	tbuff[4] = 0x00;
	tbuff[5] = 0x05;
	tbuff[6] = 0x01;
	tbuff[7] = 0x01;
	tbuff[8] = 0x00;
	tbuff[9] = 0x01;
	tbuff[10] = 0x00;
	tbuff[11] = 0x0d;
	}
        vTaskDelay(400 / portTICK_PERIOD_MS);
        uart_write_bytes(UART_NUM_0, tbuff, 12);

	}
	break;
	case 2:
	if ((bufferln[3] == 0x07) && (bufferln[5] == 0x08) && (bufferln[7] == 0x02) && (bufferln[9] == 0x04)) {
	tdata = (bufferln[10] << 24) + (bufferln[11] << 16) + (bufferln[12] << 8) + bufferln[13];
	switch (bufferln[6]) { 
	case 0x01:
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/Energy");
	bCount = tdata*10;
	bCountR = tdata*10;
	if(!bCountSign) bCount = bCount + bCountOffs;
	else bCount = bCount - bCountOffs;
	tdata = bCount / 1000;
	itoa(tdata,tbuff,10);
	strcpy(MQTT_DATA,tbuff);
	strcat(MQTT_DATA,".");
	tdata = bCount % 1000;
	tdata = tdata / 10;
	if (tdata < 10) strcat(MQTT_DATA,"0");
	itoa(tdata,tbuff,10);
	strcat(MQTT_DATA,tbuff);
	if ((bCount != bPCount) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
        bPCount = bCount;
	}
	break;
	}

	} else if ((bufferln[3] == 0x07) && (bufferln[5] == 0x0c) && (bufferln[6] == 0x06) && (bufferln[9] == 0x08)) {
	tdata = (bufferln[10] << 8) + bufferln[11];
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/Voltage");
	bVoltage = tdata;
	tdata = bVoltage / 10;
	itoa(tdata,tbuff,10);
	strcpy(MQTT_DATA,tbuff);
	strcat(MQTT_DATA,".");
	tdata = bVoltage % 10;
	itoa(tdata,tbuff,10);
	strcat(MQTT_DATA,tbuff);
	if ((bVoltage != bPVoltage) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
        bPVoltage = bVoltage;
	}
	tdata = (bufferln[12] << 16) + (bufferln[13] << 8) + bufferln[14];
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/Current");
	bCurrent = tdata;
	tdata = bCurrent / 1000;
	itoa(tdata,tbuff,10);
	strcpy(MQTT_DATA,tbuff);
	strcat(MQTT_DATA,".");
	tdata = bCurrent % 1000;
	if (tdata < 10) strcat(MQTT_DATA,"0");
	if (tdata < 100) strcat(MQTT_DATA,"0");
	itoa(tdata,tbuff,10);
	strcat(MQTT_DATA,tbuff);
	if ((bCurrent != bPCurrent) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
        bPCurrent = bCurrent;
	}
	tdata = (bufferln[15] << 16) + (bufferln[16] << 8) + bufferln[17];
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/Power");
	bPower = tdata*10;
	tdata = bPower / 10;
	itoa(tdata,tbuff,10);
	strcpy(MQTT_DATA,tbuff);
/*
	strcat(MQTT_DATA,".");
	tdata = bPower % 10;
	itoa(tdata,tbuff,10);
	strcat(MQTT_DATA,tbuff);
*/
	if ((bPower != bPPower) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
        bPPower = bPower;
	}

	} else if ((bufferln[3] == 0x07) && (bufferln[5] == 0x05) && (bufferln[6] == 0x10)) {
        bState = (bufferln[10]);
        strcpy(MQTT_TOPIC,MQTT_BASE_TOPIC);
	strcat(MQTT_TOPIC,"/state");
	if (!bState) strcpy(MQTT_DATA,strOFF);
	else strcpy(MQTT_DATA,strON);
	if ((bState != bPState) && mqttConnected) {
	esp_mqtt_client_publish(mqttclient, MQTT_TOPIC, MQTT_DATA, 0, 1, 1);
	mcmsnd = 375;
        bPState = bState;
	}
	} else if ((bufferln[3] == 0x04) && (bufferln[5] == 0x00)) {
	if (!bState) {
	tbuff[0] = 0x55;
	tbuff[1] = 0xaa;
	tbuff[2] = 0x00;
	tbuff[3] = 0x06;
	tbuff[4] = 0x00;
	tbuff[5] = 0x05;
	tbuff[6] = 0x10;
	tbuff[7] = 0x01;
	tbuff[8] = 0x00;
	tbuff[9] = 0x01;
	tbuff[10] = 0x01;
	tbuff[11] = 0x1d;
	} else {
	tbuff[0] = 0x55;
	tbuff[1] = 0xaa;
	tbuff[2] = 0x00;
	tbuff[3] = 0x06;
	tbuff[4] = 0x00;
	tbuff[5] = 0x05;
	tbuff[6] = 0x10;
	tbuff[7] = 0x01;
	tbuff[8] = 0x00;
	tbuff[9] = 0x01;
	tbuff[10] = 0x00;
	tbuff[11] = 0x1c;
	}
        vTaskDelay(400 / portTICK_PERIOD_MS);
        uart_write_bytes(UART_NUM_0, tbuff, 12);

	}

	break;
	}
}


//******************* Main **********************
void app_main()
{
	esp_err_t ret;
	wifi_ap_record_t wifidata;
//empty
//	f_update = false;
	s_retry_num = 0;
	mqttConnected = false;
	MQTT_USER[0] = 0;
	MQTT_PASSWORD[0] = 0;
	MQTT_SERVER[0] = 0;
	WIFI_SSID[0] = 0;
	WIFI_PASSWORD[0] = 0;
	bufferln[0] = 0;
	bufferlnhex[0] = 0;
	cptbufln = 0;
	cptbufrx = 0;
	lenbufrx = 0;
	tuptime = 0;
	MQTT_DATA[0] = 0;
	MQTT_TOPIC[0] = 0;
	TimeZone = 0;
	sChip = 0;
	bCountOffs = 0;
	bCountSign = 0;
	bCountR = 0;
	bCountBT = 0;
	bCountY = 0;
	CurrDay = 0;
//
	strcpy(strON,"ON");
	strcpy(strOFF,"OFF");
	strcpy(WIFI_SSID, "zmai");
	strcpy(WIFI_PASSWORD, "12345678");

//Initialize NVS
	ret = nvs_flash_init();
// read nvs
	nvs_handle my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {
	nvs_get_u32(my_handle, "scoffs", &bCountOffs);
	nvs_get_u16(my_handle, "smqprt", &mqtt_port);
	nvs_get_u8(my_handle, "sespnum", &zmainum);
	nvs_get_u8(my_handle, "timzon", &TimeZone);
	nvs_get_u8(my_handle, "schip", &sChip);
	nvs_get_u8(my_handle, "chk1",   &FDHass);
	nvs_get_u8(my_handle, "chk2",   &ftrufal);
	nvs_get_u8(my_handle,  "chk3",  &bCountSign);
	size_t nvsize = 32;
	nvs_get_str(my_handle,"swfid", WIFI_SSID,&nvsize);
	nvsize = 64;
	nvs_get_str(my_handle,"swfpsw", WIFI_PASSWORD,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"smqsrv", MQTT_SERVER,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"smqid", MQTT_USER,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"smqpsw", MQTT_PASSWORD,&nvsize);
// Close nvs
	nvs_close(my_handle);
	}
	if (!WIFI_SSID[0]) {
	strcpy(WIFI_SSID, "zmai");
	strcpy(WIFI_PASSWORD, "12345678");

	}
	vTaskDelay(500 / portTICK_PERIOD_MS);

// fill basic parameters
	char tbuff[32];
	strcpy(MQTT_BASE_TOPIC, "zmai");
	if (zmainum)  {
	itoa(zmainum,tbuff,10);
	strcat(MQTT_BASE_TOPIC, tbuff);
	}
	strcpy(MQTT_CLIENT_NAME, "zmai");
	if (zmainum)  {
	itoa(zmainum,tbuff,10);
	strcat(MQTT_CLIENT_NAME, tbuff);
	}
	if (FDHass) ftrufal = 0;
	if (ftrufal) {
	strcpy(strON,"true");
	strcpy(strOFF,"false");
	}
//Initialize hw timer
//ESP_LOGI(TAG, "Initialize hw_timer for callback");
	hw_timer_init(hw_timer_callback, NULL);
//ESP_LOGI(TAG, "Set hw_timer timing time 8ms with reload");
	hw_timer_alarm_us(8000, true);
//Initialize Wifi
	wifi_init_sta();
//Initialize Mqtt
	if (MQTT_SERVER[0]) mqtt_app_start();
//timezone
/*
to get MSK (GMT + 3) I need to write GMT-3
*/
	char tzbuf[16];
	char tzbuff[8];
	uint8_t TimZn = TimeZone;
	strcpy(tzbuf,"GMT");
	if (TimZn > 127 ) {
	strcat (tzbuf,"+");
	TimZn = ~TimZn;
	TimZn++;
	} else strcat (tzbuf,"-");
	itoa(TimZn,tzbuff,10);
	strcat(tzbuf,tzbuff);
	setenv("TZ", tzbuf, 1);
	tzset();
//sntp
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_init();
// get esp mac addr
	uint8_t macbuf[8];
        tESP8266Addr[0] = 0;
	esp_read_mac(macbuf,0);
	bin2hex(macbuf, tESP8266Addr,6);

//Initialize http server
/* Start the server for the first time */
	server = start_webserver();
// Configure parameters of an UART driver, communication pins and install the driver
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
//	uart_driver_install(UART_NUM_0, UART_BUF_SIZE * 2, 0, 0, NULL);
	uart_driver_install(UART_NUM_0, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 0, NULL);
	uart_flush_input(UART_NUM_0);
//
        vTaskDelay(1000 / portTICK_PERIOD_MS);

	char bbuff[16];
//switch on 12
	switch (sChip) {
	case 1:
	bbuff[0] = 0x55;
	bbuff[1] = 0xaa;
	bbuff[2] = 0x00;
	bbuff[3] = 0x06;
	bbuff[4] = 0x00;
	bbuff[5] = 0x05;
	bbuff[6] = 0x01;
	bbuff[7] = 0x01;
	bbuff[8] = 0x00;
	bbuff[9] = 0x01;
	bbuff[10] = 0x01;
	bbuff[11] = 0x0e;
        uart_write_bytes(UART_NUM_0, bbuff, 12);
	break;
	case 2:
	bbuff[0] = 0x55;
	bbuff[1] = 0xaa;
	bbuff[2] = 0x00;
	bbuff[3] = 0x06;
	bbuff[4] = 0x00;
	bbuff[5] = 0x05;
	bbuff[6] = 0x10;
	bbuff[7] = 0x01;
	bbuff[8] = 0x00;
	bbuff[9] = 0x01;
	bbuff[10] = 0x01;
	bbuff[11] = 0x1d;
        uart_write_bytes(UART_NUM_0, bbuff, 12);
	break;
	}
//
	int floop = 1;
//state monitoring and command execution loop
	while (floop) {

        // Read data from the UART
	if (rxlnread()) rxlnparse();

	if (tuptime - t_last > 625) {

	switch (sChip) {
	case 1:
	bbuff[0] = 0x55;
	bbuff[1] = 0xaa;
	bbuff[2] = 0x00;
	bbuff[3] = 0x08;
	bbuff[4] = 0x00;
	bbuff[5] = 0x00;
	bbuff[6] = 0x07;
        uart_write_bytes(UART_NUM_0, bbuff, 7);
	break;
	case 2:
	bbuff[0] = 0x55;
	bbuff[1] = 0xaa;
	bbuff[2] = 0x00;
	bbuff[3] = 0x08;
	bbuff[4] = 0x00;
	bbuff[5] = 0x00;
	bbuff[6] = 0x07;
        uart_write_bytes(UART_NUM_0, bbuff, 7);
	break;
	}

//	wifi_ap_record_t wifidata;
	if (esp_wifi_sta_get_ap_info(&wifidata) == 0) {
	char buft[16];
	char bufd[8];
	iRssiESP = wifidata.rssi;
	if  (mqttConnected && (iprevRssiESP != iRssiESP)) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/rssi");
        itoa(iRssiESP,bufd,10);
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
	iprevRssiESP = iRssiESP;
	}	
	}
	t_last = tuptime;
	}
	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	if (timeinfo.tm_mday != CurrDay) {
	if (CurrDay) bCountY = bCountR - bCountBT;
	bCountBT = bCountR;
	CurrDay = timeinfo.tm_mday;	
	}
	bCountT = bCountR - bCountBT;

	if ((bCountT != bPCountT) && mqttConnected) {
	uint32_t tdata;
	char tbuff[16];
	char buft[32];
	char bufd[16];
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/EnergyToday");
	tdata = bCountT / 1000;
	itoa(tdata,tbuff,10);
	strcpy(bufd,tbuff);
	strcat(bufd,".");
	tdata = bCountT % 1000;
	tdata = tdata / 10;
	if (tdata < 10) strcat(bufd,"0");
	itoa(tdata,tbuff,10);
	strcat(bufd,tbuff);
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
        bPCountT = bCountT;
	}

	if ((bCountY != bPCountY) && mqttConnected) {
	uint32_t tdata;
	char tbuff[16];
	char buft[32];
	char bufd[16];
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/EnergyYesterday");
	tdata = bCountY / 1000;
	itoa(tdata,tbuff,10);
	strcpy(bufd,tbuff);
	strcat(bufd,".");
	tdata = bCountY % 1000;
	tdata = tdata / 10;
	if (tdata < 10) strcat(bufd,"0");
	itoa(tdata,tbuff,10);
	strcat(bufd,tbuff);
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
        bPCountY = bCountY;
	}

        vTaskDelay(220 / portTICK_PERIOD_MS);
	if (s_retry_num >= WIFI_MAXIMUM_RETRY) floop = 0;

}
	esp_restart();
}
