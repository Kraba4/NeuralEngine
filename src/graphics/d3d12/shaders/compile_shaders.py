from distutils import extension
import os
import subprocess
import pathlib

if __name__ == '__main__':
    fxc_cmd = "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.22621.0\\x86\\fxc.exe"
    version_vs = "vs_5_0"
    version_ps = "ps_5_0"
    
    shader_list = ["1.vsps.hlsl", "basic.vsps.hlsl"]
    
    for shader in shader_list:
        words = shader.split('.')
        name = words[0]
        extension = words[1]
        if extension == "vs": 
            subprocess.run([fxc_cmd, shader, "/Od", "/Zi", "/T", version_vs, "/E", "VS", "/Fo", "compiled\{}vs.cso".format(name)])
        elif extension == "ps":
            subprocess.run([fxc_cmd, shader, "/Od", "/Zi", "/T", version_ps, "/E", "PS", "/Fo", "compiled\{}.ps.cso".format(name)])
        elif extension == "vsps":
            subprocess.run([fxc_cmd, shader, "/Od", "/Zi", "/T", version_vs, "/E", "VS", "/Fo", "compiled\{}.vs.cso".format(name)])
            subprocess.run([fxc_cmd, shader, "/Od", "/Zi", "/T", version_ps, "/E", "PS", "/Fo", "compiled\{}.ps.cso".format(name)])
        else:
            print("Wrong extension:", extension, "-----", shader)