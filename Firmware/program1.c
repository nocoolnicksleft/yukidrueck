

void program1_keydown(int key)
{
	if (key == programStep)
	{
	  mp3commandpush(MP3_CMD_STOP);
      mp3commandpush(MP3_CMD_LOOP_OFF);
	  mp3playtrack(fart1track);
      programState = 2;
	  programWaiting = 20;
	} else {
	}
}

void program1_start()
{
  programState = 1;
  programStep = 0;
  mp3commandpush(MP3_CMD_LOOP_ON);
}

void program1_timer100msec()
{
	button_led_off(programStep);
	if (programState == 1) // up
    {
		if (programStep == 5) 
		{
			programState = 0; // down
			programStep = 4;
		} else programStep ++;

	} else if (programState == 0) {

		if (programStep == 1) 
		{
			programState = 1; // up again
			programStep = 2;
		} else programStep --;

    } else { // stopped
		if (programWaiting == 0) 
		{
			program1_start();
		} else programWaiting--;
	}
	button_led_on(programStep);
}

void program1_keyup(int key)
{

}


void program1_stop()
{
  mp3commandpush(MP3_CMD_LOOP_OFF);
  mp3commandpush(MP3_CMD_STOP);
}

void program1_mp3commandfinished(int command)
{
  if (command == MP3_CMD_LOOP_ON) mp3playtrack(manc1track);
}

void program1_sample_started(int sample)
{

}

void program1_sample_completed(int sample)
{

}
