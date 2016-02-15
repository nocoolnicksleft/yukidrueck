

void program5_keydown(int key)
{
    if (key == 1)
    {
     if (mp3playtrack(boing1track))	{
        programState = key;
     }
	}
    else if (key == 2)
    {
     if (mp3playtrack(boing2track))	{
        programState = key;
     }
	}
    else if (key == 3)
    {
     if (mp3playtrack(boing3track))	{
        programState = key;
     }
	}
    else if (key == 4)
    {
     if (mp3playtrack(boing4track))	{
        programState = key;
     }
	}
    else if (key == 5)
    {
     if (mp3playtrack(boing5track))	{
        programState = key;
     }
	}
}

void program5_timer100msec()
{
}

void program5_keyup(int key)
{

}

void program5_start()
{

}

void program5_sample_started(int sample)
{
	button_led_on(programState);
}

void program5_sample_completed(int sample)
{
    button_led_clearall();
}

void program5_stop()
{
	mp3commandpush(MP3_CMD_STOP);
}

void program5_mp3commandfinished(int command)
{
}
