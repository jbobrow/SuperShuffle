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

byte bAutoSwap = 0; // acts like a boolean, becomes 1 when initiating auto swap
byte autoSwapFace = 6;

#define SWAP_DURATION 600
#define FADE_DURATION 300

void setup() {
  // put your setup code here, to run once:
  randomize();
  myColorIndex = random(numColors-1);
  setColor(colors[myColorIndex]);
}

void loop() {
  // put your main code here, to run repeatedly:

  // 1. handle user interaction
  if(buttonPressed()) {
    
    // Toggle from IDLE to SELECTED or reverse
    if( myState == IDLE ) {
      myState = SELECTED;
      selectedTime = millis();
    }
    else if( myState == SELECTED ) {
      myState = IDLE;
    }
  }

  // 2. handle Blinks interaction
  // look at neighbors
  // if I am IDLE... nothing to do here
  // if I am SELECTED and a neighbor is SELECTED or SWAP, I go to SWAP
  // if I am SWAP and all neighbors are SWAP or IDLE, I go to IDLE
  bool hasSelectedNeighbors = false;
  FOREACH_FACE(f) {
    if(!isValueReceivedOnFaceExpired(f)) {
      byte neighborData = getLastValueReceivedOnFace(f);
      byte neighborState = getNeighborState(neighborData);
      byte neighborColorIndex = getNeighborColorIndex(neighborData);

      if( myState == IDLE ) {
        // look for auto swap to select me
        if(getNeighborAutoSwap(neighborData) == 1) {
          myState == SELECTED;  // I've been chosen to autoswap
        }
      }
      else if ( myState == SELECTED ) {
        if( neighborState == SELECTED || neighborState == SWAP ) {
          myState = SWAP;
          swapFace = f;
          swapColorIndex = neighborColorIndex;
          swapTimer.set(SWAP_DURATION);
        }
      }
      else if ( myState == SWAP ) {
        if( neighborState == SELECTED ) {
          hasSelectedNeighbors = true;
        }
      }
    }
  }

  if ( myState == SWAP && !hasSelectedNeighbors && swapTimer.isExpired() ) {
    myState = IDLE;
    myColorIndex = swapColorIndex;
  }


  // 3. display the Blinks state
  if( myState == IDLE ) {
    setColor( colors[myColorIndex] );
  }
  else if( myState == SELECTED ) {
    displaySelected(colors[myColorIndex]);
  }
  else if( myState == SWAP ) {
    displaySwapColorsOnFace(colors[myColorIndex], colors[swapColorIndex], swapFace, SWAP_DURATION - swapTimer.getRemaining());
  }

  // 4. communicate my color and state
  // for auto swapping, we'll send unique signals on each face
  FOREACH_FACE(f) {
    // if I choose to swap    
    byte myData = (bAutoSwap << 5) + (myColorIndex << 2) + (myState);
    setValueSentOnAllFaces(myData);
  }
}

/*
 * Shows that a piece has been selected and waiting for a swap partner
 */
void displaySelected(Color c) {
  byte bri =  64 + (3 * sin8_C( 128 + (millis() - selectedTime)/6) / 4);
  setColor( dim(colors[myColorIndex], bri));
}

/*
 * a: starting color 
 * b: ending color
 * offset: face that the swap is happening on
 * t: time since animation started
 */
void displaySwapColorsOnFace(Color a, Color b, byte offset, uint16_t t) {

  byte face_shifted;
  
  FOREACH_FACE(f) {
     face_shifted = (f + offset) % 6;
     
     if(t > (f*SWAP_DURATION) / 6) {   
       setColorOnFace(b,face_shifted); 
     }
     else {
       setColorOnFace(a,face_shifted);
     }
  }  
}

byte getNeighborState(byte data) {
  return data & 3;  // return the lowest 2 bits
}

byte getNeighborColorIndex(byte data) {
  return (data >> 2) & 7;  // return the highest bits
}

byte getNeighborAutoSwap(byte data) {
  return (data >> 5) & 1; // return the 6
}
