modded class CarScript : Car {
  // clang-format off
  // Может ли машина валить деревья
  private bool MPG_FellingEnabled;
  // Какие слоты должны быть заполнены для того, что бы валилось дерево,
  // если не указано, то параметр не будет учитываться
  private ref TStringArray MPG_FellingAttachmentSlots = new TStringArray();
  // Минимальная скорость, при которой машина может валить деревья
  private float MPG_FellingMinSpeed = 10.0;
  // Минимальный импульс, при котором машина может валить деревья
  private float MPG_FellingMinImpulse = 2000.0;
  // clang-format on

  void CarScript() {
    if (ConfigIsExisting("MPG_FellingEnabled")) {
      MPG_FellingEnabled = ConfigGetBool("MPG_FellingEnabled");
    }
    if (ConfigIsExisting("MPG_FellingAttachmentSlots")) {
      ConfigGetTextArray("MPG_FellingAttachmentSlots", MPG_FellingAttachmentSlots);
    }
    if (ConfigIsExisting("MPG_FellingMinSpeed")) {
      MPG_FellingMinSpeed = ConfigGetFloat("MPG_FellingMinSpeed");
    }
    if (ConfigIsExisting("MPG_FellingMinImpulse")) {
      MPG_FellingMinImpulse = ConfigGetFloat("MPG_FellingMinImpulse");
    }
  }

  override void OnContact(string zoneName, vector localPos, IEntity other, Contact data) {

    MPG_CFT_TryToFell(other, data);

    super.OnContact(zoneName, localPos, other, data);
  }

  override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx) {
    super.OnRPC(sender, rpc_type, ctx);

    switch (rpc_type) {
    case PlantType.TREE_HARD: {
      SoundHardTreeFallingPlay();
      break;
    }

    case PlantType.TREE_SOFT: {
      SoundSoftTreeFallingPlay();
      break;
    }
    }
  }

  // clang-format off
  private void MPG_CFT_TryToFell(IEntity other, Contact data) {
    // clang-format on
    // Имеет смысл работать только на серверной стороне
    if (!g_Game.IsServer()) {
      return;
    }

    // Проверяем по очереди т.к. это по идее должно экономить ресурсы:
    // - Есть ли переменная MPG_FellingEnabled
    if (!MPG_FellingEnabled) {
      return;
    }

    // - А двигатель вообще включен?
    if (!EngineIsOn()) {
      return;
    }

    // - Не слишком ли медленно едем
    if (GetSpeedometer() < MPG_FellingMinSpeed) {
      return;
    }

    // - Не слишком ли маленький импульс
    if (data.Impulse < MPG_FellingMinImpulse) {
      return;
    }

    // - Есть ли ВСЕ требуемые аттачменты
    //   - при условии, что массив не пустой
    //   - и все аттачменты живые

    // Если слоты не указаны, считаем проверку пройденной
    bool allAttachmentsValid = true;

    if (MPG_FellingAttachmentSlots.Count() > 0) {
      foreach (string slotName : MPG_FellingAttachmentSlots) {
        ItemBase attachment = ItemBase.Cast(FindAttachmentBySlotName(slotName));
        if (!attachment || attachment.IsRuined()) {
          allAttachmentsValid = false;
          break;
        }
      }

      if (!allAttachmentsValid) {
        return;
      }
    }

    // - Дерево ли это:
    //   - существует ли такой класс
    //   - кастуется ли в базовый класс дерева
    WoodBase tree = WoodBase.Cast(other);

    if (!tree || !tree.IsTree()) {
      return;
    }

    // - Может ли оно падать
    // - Не уничтожено ли это дерево
    if (!tree.IsCuttable() || tree.IsDamageDestroyed()) {
      return;
    }

    // Ну вот теперь мы точно убедились, что все условия выполнены, можно валить дерево
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MPG_CFT_FellTree, 50, false, tree);
  }

  // clang-format off
  private void MPG_CFT_FellTree(WoodBase tree) {
    // clang-format on
    // Отправляем RPC в соответсвии с типом дерева,
    // что бы поймать потом его тут же в коде и проиграть нужный звук.
    tree.OnTreeCutDown(this);
    // Уменьшаем здоровье на максимально возможное для этого объекта.
    tree.DecreaseHealth(tree.GetMaxHealth(), tree.CanBeAutoDeleted());
  }
};