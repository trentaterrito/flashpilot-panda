from panda import Panda


class Handle:
  def __init__(self, health):
    self.health = health

  def controlRead(self, *_args, **_kwargs):
    return self.health


def panda_with_health(*, reason=0, gates=0):
  values = [0] * 23
  values[21] = reason
  values[22] = gates
  panda = object.__new__(Panda)
  panda.health_version = Panda.HEALTH_PACKET_VERSION
  panda._handle = Handle(Panda.HEALTH_STRUCT.pack(*values))
  return panda


def test_lateral_diagnostics_parse_and_default_to_zero():
  default_health = panda_with_health().health()
  assert default_health["lateral_revocation_reason"] == 0
  assert default_health["lateral_authorization_gates"] == 0

  health = panda_with_health(reason=11, gates=0xA55A).health()
  assert health["lateral_revocation_reason"] == 11
  assert health["lateral_authorization_gates"] == 0xA55A
