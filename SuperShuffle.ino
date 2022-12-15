/*
    SuperShuffle™
    by Eric Zimmerman

    Goal: get the largest cluster of the same color together by swapping color with an adjacent neighbor
    Action:
    1. click Blink to enter select mode
    2. click adjacent Blink to swap

    TODO:
      Custom color palette (bright and distinguishable colors)

    TODO:
      A.I.
      Level 1: An individual can determine which swap will result locally in more connected of the same color
      Level 2: An individual can report to neighbors it's preference for color...

    code by
    Jonathan Bobrow
    12.1.2022
    --------------------
    Blinks by Move38
    Brought to life via Kickstarter 2018

    @madewithblinks
    www.move38.com
    --------------------
*/

Color colors[] = {CYAN, ORANGE, MAGENTA, YELLOW, GREEN};
byte numColors = 5;
byte myColorIndex;

enum States {
  IDLE,
  SELECTED,
  SWAP
};

byte myState = IDLE;

uint32_t selectedTime;

byte swapFace = 6;
byte swapColorIndex = numColors;
Timer swapTimer;

bool bFirstPressed = false;

#define SWAP_DURATION 600
#define FADE_DURATION 300

void setup() {
  // put your setup code here, to run once:
  randomize();
  myColorIndex = random(numColors - 1);
  setColor(colors[myColorIndex]);
}

void loop() {
  // put your main code here, to run repeatedly:

  // 1. handle user interaction
  if (buttonPressed()) {

    // Toggle from IDLE to SELECTED or reverse
    if ( myState == IDLE ) {
      myState = SELECTED;
      selectedTime = millis();
    }
    else if ( myState == SELECTED ) {
      myState = IDLE;
    }

    if (!hasNeighborOfType(SELECTED)) {
      bFirstPressed = true;
    }

  }

  // 2. handle Blinks interaction
  // look at neighbors
  // if I am IDLE... nothing to do here
  // if I am SELECTED and a neighbor is SELECTED or SWAP, I go to SWAP
  // if I am SWAP and all neighbors are SWAP or IDLE, I go to IDLE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      byte neighborData = getLastValueReceivedOnFace(f);
      byte neighborState = getNeighborState(neighborData);
      byte neighborColorIndex = getNeighborColorIndex(neighborData);

      if ( myState == IDLE ) {
        // nothing to do here
      }
      else if ( myState == SELECTED ) {
        if ( neighborState == SELECTED || neighborState == SWAP ) {
          myState = SWAP;
          swapFace = f;
          swapColorIndex = neighborColorIndex;
          swapTimer.set(SWAP_DURATION);
        }
      }
      else if ( myState == SWAP ) {
        // nothing to do here
      }
    }
  }

  if ( myState == SWAP && !hasNeighborOfType(SELECTED) && swapTimer.isExpired() ) {
    
    bFirstPressed = false; // reset the first pressed variable
    myState = IDLE;
    myColorIndex = swapColorIndex;
  }


  // 3. display the Blinks state
  if ( myState == IDLE ) {
    setColor( colors[myColorIndex] );
  }
  else if ( myState == SELECTED ) {
    displaySelected(colors[myColorIndex]);
  }
  else if ( myState == SWAP ) {
    displaySwapColorsOnFace(colors[myColorIndex], colors[swapColorIndex], swapFace, SWAP_DURATION - swapTimer.getRemaining());
  }

  // 4. communicate my color and state
  byte myData = (myColorIndex << 2) + (myState);
  setValueSentOnAllFaces(myData);
}

/*
   Shows that a piece has been selected and waiting for a swap partner
*/
void displaySelected(Color c) {
  byte bri =  64 + (3 * sin8_C( 128 + (millis() - selectedTime) / 6) / 4);
  setColor( dim(colors[myColorIndex], bri));
}

/*
   a: starting color
   b: ending color
   offset: face that the swap is happening on
   t: time since animation started
*/
void displaySwapColorsOnFace(Color a, Color b, byte offset, uint16_t t) {

  byte face_shifted;

  // Simple version - hard transition
  FOREACH_FACE(f) {
    if (!bFirstPressed) {
      face_shifted = (f + offset) % 6;
    }
    else {
      face_shifted = ((5 - f + offset) % 6);
    }

    if (t > (f * SWAP_DURATION) / 6) {
      setColorOnFace(b, face_shifted);
    }
    else {
      setColorOnFace(a, face_shifted);
    }
  }

  // Fade slowly...
  // identify which face LED(s) are animating
  // update their status based on t
  // period for each LED = t/6
  // duration for fadeout and fadein = t/12 (if completing one at a time w/o overlap)

//  FOREACH_FACE(f) {
//    if (!bFirstPressed) {
//      face_shifted = (f + offset) % 6;
//    }
//    else {
//      face_shifted = ((5 - f + offset) % 6);
//    }
//
//    if ( int(t / (SWAP_DURATION / 6)) == f ) {
//      // this is the face that is animating
//      // progress in local animation
//      int progress = t - f * (SWAP_DURATION / 6);
//
//      // only fade out version
////      byte bri = 256 - map(progress, 0, (SWAP_DURATION / 6), 0, 256);
////      setColorOnFace(dim(a, bri), face_shifted);
//
//        // fade out fade in
////      if ( progress < (SWAP_DURATION / 12) ) {
////        // fade down color A
////        byte bri = 256 - map(progress, 0, (SWAP_DURATION / 12), 0, 256);
////        setColorOnFace(dim(a, bri), face_shifted);
////      }
////      else {
////        // fade up color B
////        byte bri = map(progress, (SWAP_DURATION / 12), (SWAP_DURATION / 6), 0, 256);
////        setColorOnFace(dim(b, progress - 256), face_shifted);
////      }
//      // DEBUG: Show blue on leading face
//      //setColorOnFace(BLUE, face_shifted);
//    }
//  }
}

/*
 * Function that returns if the Blink has any neighbors of a specific type
 */

bool hasNeighborOfType(byte type) {

    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {
        byte neighborData = getLastValueReceivedOnFace(f);
        byte neighborState = getNeighborState(neighborData);
        if ( neighborState == type ) {
          return true;
        }
      }
    }
    
    return false;
}

byte getNeighborState(byte data) {
  return data & 3;  // return the lowest 2 bits
}

byte getNeighborColorIndex(byte data) {
  return data >> 2;  // return the highest bits
}
