#include "mico.h"
#include "fogcloud_down_thread.h"
#include "fog_v2_include.h"
#include "micokit_ext.h"
#include "sensor/DHT11/DHT11.h"

#define fogcloud_down_log(M, ...) custom_log("FOG_DOWN_DATA", M, ##__VA_ARGS__)

void user_downstream_thread(mico_thread_arg_t arg);

void process_recv_data(char *data, uint32_t len);

//Ƕ��ʽjson���ݸ�ʽһ���Ǻ�app�������˵ģ�Э���û������Զ���
void process_recv_data(char *data,uint32_t len )
{
    json_object *recv_json_object = NULL,*temp_object = NULL;

    recv_json_object = json_tokener_parse((const char*)(data));

	temp_object = json_object_object_get(recv_json_object, "command_id");
	int cmd = json_object_get_int(temp_object);
	//�˴�1��ʾRGB 2��ʾ���,ƽ���ͻ������Զ���һ��Э��
	if(cmd == 1)//RGB
	{
	   temp_object = json_object_object_get(recv_json_object, "rgb_led_h");
	   int h = json_object_get_int(temp_object);
	   temp_object = json_object_object_get(recv_json_object, "rgb_led_s");
	   int s = json_object_get_int(temp_object);
	   temp_object = json_object_object_get(recv_json_object, "rgb_led_b");
	   int b = json_object_get_int(temp_object);
	   hsb2rgb_led_open(h,s,b);
	   
	}
	else if(cmd == 2)//���
	{
	    temp_object = json_object_object_get(recv_json_object, "motor_switch");
	    bool motor_switch = json_object_get_boolean(temp_object);
		if(motor_switch == true)
		{
		   dc_motor_set(1);
		}
		else
		{
		   dc_motor_set(0);
		}
	}
	

    json_object_put(recv_json_object);    //ֻҪfree�����Ǹ��Ϳ���
    recv_json_object=NULL;

}


//�û����������߳�
void user_downstream_thread( mico_thread_arg_t arg )
{
    OSStatus err = kUnknownErr;
    char *fog_recv_buff = NULL;

    fog_recv_buff = malloc( FOG_V2_RECV_BUFF_LEN );
    require( fog_recv_buff != NULL, exit );

    fogcloud_down_log("------------downstream_thread start------------");

    while ( 1 )
    {
        memset(fog_recv_buff, 0, FOG_V2_RECV_BUFF_LEN);
        err = fog_v2_device_recv_command( fog_recv_buff, FOG_V2_RECV_BUFF_LEN, MICO_NEVER_TIMEOUT );
        if ( err == kNoErr )
        {
            //��������
            fogcloud_down_log("user_downstream_thread recv:%s", fog_recv_buff);
            process_recv_data( fog_recv_buff, strlen(fog_recv_buff ) );    //��������
        }

    }

    exit:
    if ( fog_recv_buff != NULL )
    {
        free( fog_recv_buff );
        fog_recv_buff = NULL;
    }

    if ( kNoErr != err )
    {
        fogcloud_down_log("ERROR: user_downstream_thread exit with err=%d", err);
    }
    mico_rtos_delete_thread( NULL );  // delete current thread

    return;
}
