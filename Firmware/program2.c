
void playnote(int key)
{
	button_led_clearall();
	if (key == 5) {
		mp3playtrack(ton_c_track);
    }
    else if (key == 4) {
		mp3playtrack(ton_d_track);
	}
    else if (key == 3) {
		mp3playtrack(ton_e_track);
	}
    else if (key == 2) {
		mp3playtrack(ton_f_track);
	}
    else if (key == 1) {
		mp3playtrack(ton_g_track);
	}
	button_led_on(key);
}

void program2_keydown(int key)
{
	playnote(key);
}



void program2_timer100msec()
{
	
}

void program2_keyup(int key)
{
	mp3commandpush(MP3_CMD_STOP);
	button_led_clearall();
}

void program2_start()
{
	mp3commandpush(MP3_CMD_LOOP_ON);
}

void program2_sample_started(int sample)
{

}

void program2_sample_completed(int sample)
{

}

void program2_stop()
{
  mp3commandpush(MP3_CMD_LOOP_OFF);
  mp3commandpush(MP3_CMD_STOP);
}

void program2_mp3commandfinished(int command)
{
}
