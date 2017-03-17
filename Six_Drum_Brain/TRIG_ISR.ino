// modified from Neutron Sound version (I'm only using 6 outputs, not all 12)
void FASTRUN ISR_TRIGEND(void){
  for (int z = 0;z <6;z++){
    triggerTimer[z] ++;
    if (triggerTimer[z] == trigLength){    
    if (out2pin[z]){  
    analogWrite(out2pin[z],0);
    }
    else analogWrite(A14,0);
    }
  }
}
