
void program7_keydown(int key)
{
	char title[20];
	sprintf(title, "lied_%u_%u", program_running,key);
	if (mp3playtrack(title))	{
    	programState = key;
    }
}

void program7_timer100msec()
{
}

void program7_keyup(int key)
{

}

void program7_start()
{

}

void program7_sample_started(int sample)
{
	button_led_on(programState);
}

void program7_sample_completed(int sample)
{
	button_led_clearall();
}

void program7_stop()
{
	mp3commandpush(MP3_CMD_STOP);
}

void program7_mp3commandfinished(int command)
{

}
