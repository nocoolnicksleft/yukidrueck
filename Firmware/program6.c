

void program6_keydown(int key)
{
    if (key == 1)
    {
     if (mp3playtrack(bell1track))	{
        programState = key;
     }
	}
    else if (key == 2)
    {
     if (mp3playtrack(bell2track))	{
        programState = key;
     }
	}
    else if (key == 3)
    {
     if (mp3playtrack(bell3track))	{
        programState = key;
     }
	}
    else if (key == 4)
    {
     if (mp3playtrack(bell4track))	{
        programState = key;
     }
	}
    else if (key == 5)
    {
     if (mp3playtrack(bell5track))	{
        programState = key;
     }
	}
}

void program6_timer100msec()
{
}

void program6_keyup(int key)
{

}

void program6_start()
{

}

void program6_sample_started(int sample)
{
	button_led_on(programState);
}

void program6_sample_completed(int sample)
{
    button_led_clearall();
}

void program6_stop()
{
	mp3commandpush(MP3_CMD_STOP);
}

void program6_mp3commandfinished(int command)
{
}
