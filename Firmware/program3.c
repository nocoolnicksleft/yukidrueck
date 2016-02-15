

int sequence[10] = { 4,2,1,5,3,1,3,4,2,5 };
int sequencecurrent;

void program3_start()
{
  mp3commandpush(MP3_CMD_LOOP_OFF);
	  
  programState = 1;
  programStep = 0;
  sequencecurrent = 0;
/*
  int i;
  for (i = 0; i < 10; i++)
  {
	sequence[i] = rand();
  }
*/
  playnote(sequence[sequencecurrent]);
}

void program3_keydown(int key)
{
	if (programState == 2)
	{
if (!mp3playing) {
		if (key == sequence[sequencecurrent])
		{
			sequencecurrent++;
			playnote(key);

		} else {
			
			mp3playtrack(fart1track);
			programState = 3;
		}
}
	}
}

void program3_timer100msec()
{
	if (programWaiting > 0)
	{
		if (programWaiting == 1)
		{
			if (programState == 1) { // playback
				playnote(sequence[sequencecurrent]);
			} else if (programState == 2) { // read
    		} else if (programState == 3) { // error
			} else if (programState == 4) { // finished
				program3_start();
			}

			programWaiting = 0;
		}  else programWaiting--;

	}

}

void program3_sample_completed(int sample)
{
	button_led_clearall();

	if (programState == 1) // play
	{
		if (sequencecurrent < programStep)
		{
			sequencecurrent++;
  			playnote(sequence[sequencecurrent]);
  		} else {
			programState = 2;
			sequencecurrent = 0;
		}
	} else if (programState == 2) { // read

		if (sequencecurrent <= programStep) {
		//	sequencecurrent++;
		} else {
			if (programStep == 5) {
				mp3playtrack(bell1track);
				programState = 4;
				programWaiting = 20;
			} else {
				programState = 1;
				programStep++;
				sequencecurrent = 0;
				programWaiting = 7;
  				
			}
  		}
		
	}

}

void program3_keyup(int key)
{
}


void program3_sample_started(int sample)
{

}
void program3_stop()
{
	mp3commandpush(MP3_CMD_STOP);
}

void program3_mp3commandfinished(int command)
{
}
