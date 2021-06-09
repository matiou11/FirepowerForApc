class FP_ScoreManager
{
  private:
    int score;
    int bonus;
    int firePowerBonus;
    byte bonusMultiplier;
    bool enabled;
    bool firepowerBonusCollected;
    bool specialLit;
    bool specialCollected;
    APC_LampBank *bonusLamps;
    APC_LampBank *multipliersLamps;
  public:
    FP_ScoreManager()
    {
      bonusLamps = new APC_LampBank(9, FP_LP_PLAYFIELD_BONUS);
      multipliersLamps = new APC_LampBank (4, FP_LP_MULTIPLIERS);
      Enable(false);
      Reset();
    }
    void Enable(bool enable)
    {
      enabled = enable;
      bonusLamps->Enable(enable);
      multipliersLamps->Enable(enable);
    }
    void Reset()
    {
      bonus = 0;
      score = 0;
      firePowerBonus = 10000;
      specialLit = false;
      specialCollected = false;
      firepowerBonusCollected = false;
      bonusLamps->Reset();
      multipliersLamps->Reset();
      UpdateLights();
    }
    void Add1000ToBonus()
    {
      if (bonus <= 28000) // maxed out ?
      {
        bonus += 1000;
        UpdateLights();
      }
    }
    void IncrementScore(int points)
    {
      score += points;
    }
    int BonusPoints()
    {
      return bonus;
    }
    int Score()
    {
      return score;
    }
    bool IncrementBonusMultiplier() // returns falls when maxed out
    {
      if (bonusMultiplier < 5)
      {
        bonusMultiplier++;
        multipliersLamps->TurnOneOn();
        return true;
      }
      else
      {
        return false;
      }
    }
    int FirepowerBonus()
    {
      return firePowerBonus;
    }
    void CollectAndIncrementFirepowerBonus()
    {
      if (!firepowerBonusCollected) score += firePowerBonus;
      if (firePowerBonus < 50000)
      {
        if (firePowerBonus == 10000) firePowerBonus == 30000;
        else if (firePowerBonus == 30000) firePowerBonus == 50000;
      }
      else
      {
        firepowerBonusCollected = true;
        if (specialLit && !specialCollected)
        {
          // 100000 points ?
          score += 100000;
          specialCollected = true;
        }
        else
        {
          specialLit = !specialCollected;
        }
      }      
      UpdateLights();
    }
    void UpdateLights()
    {
      if (enabled)
      {
        bonusLamps->TurnSomeLampsOn(bonus%10000 / 1000);
        if (bonus >= 10000) TurnOnLamp(FP_LP_PLAYFIELD_BONUS_10000); else TurnOffLamp(FP_LP_PLAYFIELD_BONUS_10000);
        if (bonus >= 20000) TurnOnLamp(FP_LP_PLAYFIELD_BONUS_20000); else TurnOffLamp(FP_LP_PLAYFIELD_BONUS_20000);
        if (firePowerBonus >= 10000) TurnOnLamp(FP_LP_FIREPOWER_BONUS_10000); else TurnOffLamp(FP_LP_FIREPOWER_BONUS_10000);
        if (firePowerBonus >= 30000) TurnOnLamp(FP_LP_FIREPOWER_BONUS_30000); else TurnOffLamp(FP_LP_FIREPOWER_BONUS_30000);
        if (firePowerBonus >= 50000) TurnOnLamp(FP_LP_FIREPOWER_BONUS_50000); else TurnOffLamp(FP_LP_FIREPOWER_BONUS_50000);
        if (specialLit)
        {
          TurnOnLamp(FP_LP_SPECIAL_LEFT); 
          TurnOnLamp(FP_LP_SPECIAL_RIGHT); 
        }
        else 
        {
          TurnOffLamp(FP_LP_SPECIAL_LEFT);
          TurnOffLamp(FP_LP_SPECIAL_RIGHT);
        }
        multipliersLamps->UpdateLights();
      }
    }
};
