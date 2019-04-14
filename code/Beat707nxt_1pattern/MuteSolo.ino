/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2019 - http://www.Beat707.com
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void createScreenMute()
{
  resetSegments(1, 2);
  //
  byte xseg = 0;
  byte xboard = 1;
  for (byte x=0; x<(DRUM_TRACKS + NOTE_TRACKS); x++)
  {
    if (bitRead(configData.muteTrack, x) == 1) segments[xboard][xseg] = S_MUTE; else segments[xboard][xseg] = S_UNMUTE;
    //
    xseg++;
    if (xseg >= 8)
    {
      xseg = 0;
      xboard++;
    }
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void checkInterfaceMute()
{
  for (byte x=0; x<8; x++)
  {
    for (byte i=0; i<2; i++)
    {
      if (buttonEvent[i + 1][x] >= kButtonClicked)
      {
        if (buttonEvent[i + 1][x] >= kButtonClicked)
        {
          if (bitRead(configData.muteTrack, x + (i *8)) == 1) bitClear(configData.muteTrack, x + (i *8)); else bitSet(configData.muteTrack, x + (i *8));
        }
      }
      //
      buttonEvent[i + 1][x] = 0;
      updateScreen = true;
    }
  }  
  //
  if (buttonEvent[0][7] >= kButtonClicked)
  {
    curRightScreen = kRightSteps;
    updateScreen = true;
    memset(buttonEvent, 0, sizeof(buttonEvent));
  }
}
