// Firepower classes

// Fire lanes "F" "I" "R" "E"
class FP_FireLanes
{
  private:
    bool laneStatus[FP_NB_FIRE_LANES];
  public:
    FP_FireLanes()
    {
      ResetLanes();
    }
    void ResetLanes()
    {
      for ( int i = 0 ; i < FP_NB_FIRE_LANES ; i++ ) laneStatus[i] = false;
    }
    void RollOverActivated( byte switchNumber )
    {     
      for ( int i = 0 ; i < FP_NB_FIRE_LANES ; i++ )
        if (FP_SW_FIRE_ROLLOVERS[i] == switchNumber) laneStatus[i] = true;
    }
    bool AllLanesOn()
    {
      bool allLanesOn = true;
      for ( int i = 0 ; i < FP_NB_FIRE_LANES ; i++ )
        if (laneStatus[i] == false) allLanesOn = false;
      return allLanesOn;
    }
    void LaneChange() // shift all values to the right
    {
      bool last = laneStatus[FP_NB_FIRE_LANES - 1];
      for ( int i = FP_NB_FIRE_LANES - 1 ; i > 0  ; i-- ) laneStatus[i] = laneStatus[i-1];
      laneStatus[0] = last;
    }
};
