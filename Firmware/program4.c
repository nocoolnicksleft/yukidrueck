
void program4_keydown(int key)
{
	
	if (key == 2)
    {
     if (mp3playtrack(mootrack))	{
        programState = key;
     }
	}
    else if (key == 1)
    {
     if (mp3playtrack(donkeytrack))	{
        programState = key;
     }
	}
    else if (key == 4)
    {
     if (mp3playtrack(dogtrack))	{
        programState = key;
     }
	}
    else if (key == 3)
    {
     if (mp3playtrack(horsetrack))	{
        programState = key;
     }
	}
    else if (key == 5)
    {
     if (mp3playtrack(sheeptrack))	{
        programState = key;
     }
	}
}

void program4_timer100msec()
{
}

void program4_keyup(int key)
{

}

void program4_start()
{

}

void program4_sample_started(int sample)
{
	button_led_on(programState);
}

void program4_sample_completed(int sample)
{
	button_led_clearall();
}

void program4_stop()
{
	mp3commandpush(MP3_CMD_STOP);
}

void program4_mp3commandfinished(int command)
{

}
