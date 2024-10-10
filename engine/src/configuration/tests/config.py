import yaml
import vegastrike_python


def get_config():
    with open('config.yaml', 'r') as file:
        yaml_file = yaml.safe_load(file)
        print(yaml_file)

        graphics = vegastrike_python.GraphicsConfig()
        graphics.screen = yaml_file['graphics']['screen']
        graphics.resolution_x = yaml_file['graphics']['resolution_x']
        graphics.resolution_y = yaml_file['graphics']['resolution_y']
        
    
    return graphics


#gfx = get_config()
#print('screen: ', gfx.screen)
#print('x: ', gfx.resolution_x)
#print('y: ', gfx.resolution_y)


