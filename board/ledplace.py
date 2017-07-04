
import pcbnew
import math

board = pcbnew.GetBoard()

center = (4.0, -4.0)
radius = 1.2

c_offset  = (0.25, -0.04)
v1_offset = (0.175, 0.075)
v2_offset = (0.175, -0.075)
v3_offset = (0.025, -0.05)
v4_offset = (-0.175, -0.025)

c_off_angle = math.degrees(math.atan( c_offset[0] / (radius + c_offset[1]) ))
c_radius = c_offset[0] / math.sin(math.radians(c_off_angle))

v1_off_angle = math.degrees(math.atan( v1_offset[0] / (radius + v1_offset[1]) ))
v1_radius = v1_offset[0] / math.sin(math.radians(v1_off_angle))
v2_off_angle = math.degrees(math.atan( v2_offset[0] / (radius + v2_offset[1]) ))
v2_radius = v2_offset[0] / math.sin(math.radians(v2_off_angle))
v3_off_angle = math.degrees(math.atan( v3_offset[0] / (radius + v3_offset[1]) ))
v3_radius = v3_offset[0] / math.sin(math.radians(v3_off_angle))
v4_off_angle = math.degrees(math.atan( v4_offset[0] / (radius + v4_offset[1]) ))
v4_radius = v4_offset[0] / math.sin(math.radians(v4_off_angle))


angle = 90
for i in range(12):
  lednum = i+1
  cangle = (angle - c_off_angle) % 360
  v1angle = (angle - v1_off_angle) % 360
  v2angle = (angle - v2_off_angle) % 360
  v3angle = (angle - v3_off_angle) % 360
  v4angle = (angle - v4_off_angle) % 360
  ledpos = (round(center[0] + (math.cos(math.radians(angle))*radius), 3), 
            round(center[1] + (math.sin(math.radians(angle))*radius), 3))
  cpos = (round(center[0] + (math.cos(math.radians(cangle))*c_radius), 3),
          round(center[1] + (math.sin(math.radians(cangle))*c_radius), 3))

  v1pos = (round(center[0] + (math.cos(math.radians(v1angle))*v1_radius), 3),
           round(center[1] + (math.sin(math.radians(v1angle))*v1_radius), 3))
  v2pos = (round(center[0] + (math.cos(math.radians(v2angle))*v2_radius), 3),
           round(center[1] + (math.sin(math.radians(v2angle))*v2_radius), 3))
  v3pos = (round(center[0] + (math.cos(math.radians(v3angle))*v3_radius), 3),
           round(center[1] + (math.sin(math.radians(v3angle))*v3_radius), 3))
  v4pos = (round(center[0] + (math.cos(math.radians(v4angle))*v4_radius), 3),
           round(center[1] + (math.sin(math.radians(v4angle))*v4_radius), 3))

  ledrot = (angle - 90) % 360
  crot = cangle

  led = board.FindModuleByReference("LED%d" % lednum)
  led.SetPosition(pcbnew.wxPoint(ledpos[0] * 25400000.0, ledpos[1] * -25400000.0))
  led.SetOrientation((ledrot + 180) * 10)

  cap = board.FindModuleByReference("LEDC%d" % lednum)
  cap.SetPosition(pcbnew.wxPoint(cpos[0] * 25400000.0, cpos[1] * -25400000.0))
  cap.SetOrientation((crot + 180) * 10)

  #f.write("via 'LED_DATA%d' (%.3f %.3f)\n" % (lednum, v1pos[0], v1pos[1]))
  #f.write("via 'VCC' (%.3f %.3f)\n" % (v2pos[0], v2pos[1]))
  #f.write("via 'VCC' (%.3f %.3f)\n" % (v3pos[0], v3pos[1]))
  #f.write("via 'LED_DATA%d' (%.3f %.3f)\n" % (lednum-1, v4pos[0], v4pos[1]))

  angle = (angle - 30) % 360

