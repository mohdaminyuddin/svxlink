/**
@file	 AudioPaser.cpp
@brief   A_brief_description_for_this_file
@author  Tobias Blomberg / SM0SVX
@date	 2004-04-03

A_detailed_description_for_this_file

\verbatim
<A brief description of the program or library this file belongs to>
Copyright (C) 2003 Tobias Blomberg / SM0SVX

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
\endverbatim
*/



/****************************************************************************
 *
 * System Includes
 *
 ****************************************************************************/

#include <stdio.h>

#include <algorithm>


/****************************************************************************
 *
 * Project Includes
 *
 ****************************************************************************/

#include <AsyncTimer.h>


/****************************************************************************
 *
 * Local Includes
 *
 ****************************************************************************/

#include "AudioPaser.h"



/****************************************************************************
 *
 * Namespaces to use
 *
 ****************************************************************************/

using namespace std;
using namespace Async;



/****************************************************************************
 *
 * Defines & typedefs
 *
 ****************************************************************************/



/****************************************************************************
 *
 * Local class definitions
 *
 ****************************************************************************/



/****************************************************************************
 *
 * Prototypes
 *
 ****************************************************************************/



/****************************************************************************
 *
 * Exported Global Variables
 *
 ****************************************************************************/




/****************************************************************************
 *
 * Local Global Variables
 *
 ****************************************************************************/



/****************************************************************************
 *
 * Public member functions
 *
 ****************************************************************************/


AudioPaser::AudioPaser(int sample_rate, int block_size, int prebuf_time)
  : sample_rate(sample_rate), buf_size(block_size), prebuf_time(prebuf_time),
    buf_pos(0), do_flush(false)
{
  buf = new short[buf_size];
  prebuf_samples = prebuf_time * sample_rate / 1000;
} /* AudioPaser::AudioPaser */


AudioPaser::~AudioPaser(void)
{
  delete [] buf;
} /* AudioPaser::~AudioPaser */


int AudioPaser::audioInput(short *samples, int count)
{
  int samples_written = 0;
  
  if (prebuf_samples > 0)
  {
    samples_written = count;
    prebuf_samples -= samples_written;
    if (prebuf_samples <= 0)
    {
      //printf("Prebuffering done...\n");
      samples_written += prebuf_samples;
      samples_written = audioOutput(samples, samples_written);
      samples_written += audioInput(samples + samples_written,
      	      	      	      	    count - samples_written);
      pase_timer = new Timer(buf_size * 1000 / sample_rate,
      	      	      	     Timer::TYPE_PERIODIC);
      pase_timer->expired.connect(slot(this, &AudioPaser::outputNextBlock));
    }
    else
    {
      	// FIXME: Take care of the case where not all samples were written
      audioOutput(samples, samples_written);
    }
  }
  else
  {
    samples_written = min(count, buf_size - buf_pos);
    memcpy(buf + buf_pos, samples, samples_written * sizeof(short));
    buf_pos += samples_written;
    if (buf_pos == buf_size)
    {
      audioInputBufFull(true);
    }
  }
  
  //printf("%d samples of %d written into paser\n", samples_written, count);
  
  return samples_written;
  
} /* AudioPaser::audioInput */


void AudioPaser::flushAllAudio(void)
{
  if (buf_pos == 0)
  {
    allAudioFlushed();
  }
  else
  {
    do_flush = true;
  }
} /* AudioPaser::flushAllAudio */



/****************************************************************************
 *
 * Protected member functions
 *
 ****************************************************************************/


/*
 *------------------------------------------------------------------------
 * Method:    
 * Purpose:   
 * Input:     
 * Output:    
 * Author:    
 * Created:   
 * Remarks:   
 * Bugs:      
 *------------------------------------------------------------------------
 */






/****************************************************************************
 *
 * Private member functions
 *
 ****************************************************************************/


/*
 *----------------------------------------------------------------------------
 * Method:    
 * Purpose:   
 * Input:     
 * Output:    
 * Author:    
 * Created:   
 * Remarks:   
 * Bugs:      
 *----------------------------------------------------------------------------
 */
void AudioPaser::outputNextBlock(Timer *t)
{
  //printf("Timer expired\n");
  if (buf_pos < buf_size)
  {
    delete pase_timer;
    pase_timer = 0;
    prebuf_samples = prebuf_time * sample_rate / 1000;
    //printf("AudioPaser::outputNextBlock: Turning on prebuffering...\n");
  }
  
    // FIXME: Take care of the case where not all samples were written
  audioOutput(buf, buf_pos);
  buf_pos = 0;
  audioInputBufFull(false);
  
  if (do_flush)
  {
    allAudioFlushed();
    do_flush = false;
  }
  
} /* AudioPaser::outputNextBlock */






/*
 * This file has not been truncated
 */

