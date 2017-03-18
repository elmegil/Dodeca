// modified from Neutron Sound version: 
//  -- I'm only using 6 outputs, not all 12
//  -- combined increment and if()
//  -- added debounce timer
void FASTRUN ISR_TRIGEND(void){
  for (int z = 0;z <6;z++){
    if (++triggerTimer[z] == TRIGLENGTH){    
      PINWRITE(z,0);
    }
    if (++debounceTimer[z] == BOUNCETIME) {
      debounceReady[z] = true;
    }
  }
}
