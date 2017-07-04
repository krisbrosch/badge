import pcbnew
import math

center = (4.0, -4.0)
radius = 1.2

class RelativePoint:
  def __init__(self, off_x, off_y, rot_off = 0):
    self.off_x = off_x
    self.off_y = off_y
    self.rot_off = rot_off
    if (self.off_x == 0.0) and (self.off_y == 0.0):
      self.off_angle = 0
      self.radius = radius
    else:
      self.off_angle = math.degrees(math.atan( self.off_x / (radius + self.off_y) ))
      self.radius = self.off_x / math.sin(math.radians(self.off_angle))

  def angle(self, angle):
    return (angle - self.off_angle) % 360

  def pos(self, angle):
    pos = (round(center[0] + (math.cos(math.radians(self.angle(angle)))*self.radius), 3),
           round(center[1] + (math.sin(math.radians(self.angle(angle)))*self.radius), 3))
    return pos

  def kipoint(self, angle):
    pos = self.pos(angle)
    return pcbnew.wxPoint(pos[0] * 25400000.0, pos[1] * -25400000.0)

  def kirot(self, angle):
    return ((angle + 180 - self.off_angle - self.rot_off) % 360) * 10

ledpt = RelativePoint(0.0, 0.0, 90)
cpt  = RelativePoint(0.25, -0.04)
cppt = RelativePoint(0.242, -0.076)
v1pt = RelativePoint(0.175, 0.075)
v2pt = RelativePoint(0.175, -0.075)
v3pt = RelativePoint(0.025, -0.05)
v4pt = RelativePoint(-0.175, -0.025)
p1pt = RelativePoint(0.0965, -0.0630)
p2pt = RelativePoint(0.0965, 0.0630)
p3pt = RelativePoint(-0.0965, 0.0630)
p4pt = RelativePoint(-0.0965, -0.0630)

board = pcbnew.GetBoard()
top_layer = board.GetLayerID("F.Cu")
bottom_layer = board.GetLayerID("B.Cu")

nets = board.GetNetsByName()

angle = 90
lastAngle = 120
for i in range(12):
  lednum = i+1

  led = board.FindModuleByReference("LED%d" % lednum)
  led.SetPosition(ledpt.kipoint(angle))
  led.SetOrientation(ledpt.kirot(angle))

  cap = board.FindModuleByReference("LEDC%d" % lednum)
  cap.SetPosition(cpt.kipoint(angle))
  cap.SetOrientation(cpt.kirot(angle))

  if lednum < 12:
    netname = "/LEDs/LED_DATA%d" % (lednum)

    # Pin 2 -> Via 1
    # (reaching forward towards the next led)

    track = pcbnew.TRACK(board)
    track.SetStart(p2pt.kipoint(angle))
    track.SetEnd(v1pt.kipoint(angle))
    track.SetWidth(254000)
    track.SetLayer(top_layer)
    board.Add(track)
    track.SetNet(nets[netname])

    via = pcbnew.VIA(board)
    board.Add(via)
    via.SetLayerPair(top_layer, bottom_layer)
    via.SetPosition(v1pt.kipoint(angle))
    via.SetWidth(685800)
    via.SetNet(nets[netname])

  if lednum > 1:
    netname = "/LEDs/LED_DATA%d" % (lednum - 1)

    # Pin 4 -> Via 4
    # (reaching back towards the previous led)

    track = pcbnew.TRACK(board)
    track.SetStart(p4pt.kipoint(angle))
    track.SetEnd(v4pt.kipoint(angle))
    track.SetWidth(254000)
    track.SetLayer(top_layer)
    board.Add(track)
    track.SetNet(nets[netname])

    via = pcbnew.VIA(board)
    board.Add(via)
    via.SetLayerPair(top_layer, bottom_layer)
    via.SetPosition(v4pt.kipoint(angle))
    via.SetWidth(685800)
    via.SetNet(nets[netname])

    # Via 4 -> Previous Via 1
    # (connecting to the previous led's via on the bottom)

    track = pcbnew.TRACK(board)
    track.SetStart(v4pt.kipoint(angle))
    track.SetEnd(v1pt.kipoint(lastAngle))
    track.SetWidth(254000)
    track.SetLayer(bottom_layer)
    board.Add(track)
    track.SetNet(nets[netname])

  netname = "+BATT"

  # Pin 1 -> Via 2
  # (power forward)

  track = pcbnew.TRACK(board)
  track.SetStart(p1pt.kipoint(angle))
  track.SetEnd(v2pt.kipoint(angle))
  track.SetWidth(610000)
  track.SetLayer(top_layer)
  board.Add(track)
  track.SetNet(nets[netname])

  via = pcbnew.VIA(board)
  board.Add(via)
  via.SetLayerPair(top_layer, bottom_layer)
  via.SetPosition(v2pt.kipoint(angle))
  via.SetWidth(685800)
  via.SetNet(nets[netname])

  # Via 2 -> Capacitor pad
  # (power to the capacitors)

  track = pcbnew.TRACK(board)
  track.SetStart(v2pt.kipoint(angle))
  track.SetEnd(cppt.kipoint(angle))
  track.SetWidth(610000)
  track.SetLayer(top_layer)
  board.Add(track)
  track.SetNet(nets[netname])

  # Pin 1 -> Via 3
  # (power back)

  track = pcbnew.TRACK(board)
  track.SetStart(p1pt.kipoint(angle))
  track.SetEnd(v3pt.kipoint(angle))
  track.SetWidth(610000)
  track.SetLayer(top_layer)
  board.Add(track)
  track.SetNet(nets[netname])

  via = pcbnew.VIA(board)
  board.Add(via)
  via.SetLayerPair(top_layer, bottom_layer)
  via.SetPosition(v3pt.kipoint(angle))
  via.SetWidth(685800)
  via.SetNet(nets[netname])

  # Via 3 -> Previous Via 2
  # (connect power back)

  track = pcbnew.TRACK(board)
  track.SetStart(v3pt.kipoint(angle))
  track.SetEnd(v2pt.kipoint(lastAngle))
  track.SetWidth(610000)
  track.SetLayer(bottom_layer)
  board.Add(track)
  track.SetNet(nets[netname])

  lastAngle = angle
  angle = (angle - 30) % 360

