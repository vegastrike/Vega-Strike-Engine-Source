import VS
import GUI
import custom
import Base
import debug

text_height=0.1

def parse_dialog_box(args):
	i=0
	elementsToCreate=[]
	currentList=elementsToCreate
	while i<len(args):
		type=args[i]
		i+=1
		if type=='list':
			id=str(args[i])
			jlen=int(args[i+1])
			i+=2
			listitems=args[i:i+jlen]
			i+=jlen
			currentList.append(DialogBox.List(id,listitems))
		elif type=='space' or type=='width' or type=='height':
			if type=='height':
				wid=0.
			else:
				wid=float(args[i])
				i+=1
			if type=='width':
				hei=0.
			else:
				hei=float(args[i])
				i+=1
			currentList.append(DialogBox.Space(wid,hei))
		elif type=='button' or type=='buttonspr':
			id=str(args[i])
			i+=1
			if type=='buttonspr':
				sprite=str(args[i])
				width=float(args[i+1])
				height=float(args[i+2])
				i+=3
				name=id
			else:
				name=id #args[i]
				#i+=1
				width=0.
				height=text_height
				sprite=None
			currentList.append(DialogBox.Button(id,name,sprite,width,height))
		elif type=='text' or type=="textwidth":
			text=str(args[i])
			i+=1
			wid=0.
			if type=="textwidth":
				wid=float(args[i])
				i+=1
			currentList.append(DialogBox.Text(text,wid))
		elif type=='textinput':
			id=str(args[i])
			initialvalue=str(args[i+1])
			i+=2
			currentList.append(DialogBox.TextInput(id,initialvalue))
		elif type=='endrow':
			currentList=elementsToCreate
		elif type=='row':
			currentList=[]
			elementsToCreate.append(DialogBox.Row(currentList))
		else:
			debug.debug("Unknown dialog item type "+str(type))
	return elementsToCreate

if not VS.isserver():
	font_size = 1.0
	text_height = Base.GetTextHeight('|',tuple([font_size]*3))*1.05
	list_height = 0.0
	screen_color = GUI.GUIColor(200/255.0, 220/255.0 ,210/255.0)		# first I tried rgb(56 60 24) and rgb(40 44 20); both were too light
	input_color = GUI.GUIColor(25/255.0, 25/255.0 ,75/255.0)
	screen_bgcolor = GUI.GUIColor(0.9,0.9,1.0,0.7)
	screen_bgcolor_nc = GUI.GUIColor(0.1,0.0,0.4)

def makeRect(x,y,wid,hei):
	return GUI.GUIRect(x,y,wid,hei,'normalized_biased_scaled')

class DialogBox:
	class Item:
		next_id=0
		def __init__(self,id=None):
			self.gui_element=None
			numid=DialogBox.Item.next_id=DialogBox.Item.next_id+1
			numid=str(numid)
			id = id or ''
			self.idname=id+"_"+numid
			id = id or numid
			self.id=id
		def height(self):
			return 0.
		def width(self):
			return 0.
		def getValues(self,values):
			pass
		def create(self,owner,room,x,y,wid,hei):
			return None
		def draw(self):
			#print 'Drawing element',self.idname
			if self.gui_element:
				self.gui_element.show()
				self.gui_element.draw()
		def undraw(self):
			if self.gui_element:
				self.gui_element.hide()
				self.gui_element.undraw()
	class Space(Item):
		def __init__(self,width,height):
			self.hei=height
			self.wid=width
			DialogBox.Item.__init__(self)
		def height(self):
			return self.hei
		def width(self):
			return self.wid
	class List(Item):
		def __init__(self,id,items):
			self.items=items
			self.numlines=len(items)
			self.scroll=False
			if self.numlines>10:
				self.scroll=True
				self.numlines=10
			DialogBox.Item.__init__(self,id)
		def height(self):
			return (self.numlines+1) * (text_height+0.005) #fudge factor
		def getValues(self,values):
			values.append(self.id)
			sel = self.picker.selection
			if sel is None:
				values.append('')
			else:
				values.append(self.picker.items[sel].data)
		def scrollUp(self,button,params):
			self.picker.scroll('up')
		def create(self,owner,room,x,y,wid,hei):
			screen_loc = makeRect(x,y-.5*text_height,wid-0.05,hei-text_height)
			self.gui_element=GUI.GUIGroup(room)

			self.picker=GUI.GUISimpleListPicker(room,'XXXPicker',self.idname+'p', screen_loc,
				textcolor    =input_color     , textbgcolor    =screen_bgcolor,
				selectedbgcolor=screen_bgcolor_nc, selectedcolor=screen_color, owner=owner,
				textfontsize =font_size)
			self.picker.items=[ GUI.GUISimpleListPicker.listitem(el,el) for el in self.items ]
			self.gui_element.children.append(self.picker)
			
			if self.scroll:
				screen_loc = makeRect(x+wid-0.05, y, 0.05, text_height)
				scroll_up=GUI.GUIButton(room,'Scroll Up',self.idname+'u',{'*':None},screen_loc,
					clickHandler=lambda b,p:self.picker.viewMove(-3),textfontsize=font_size)
				scroll_up.setCaption(' /\\ ')
				self.gui_element.children.append(scroll_up)

				screen_loc = makeRect(x+wid-0.05, y-hei+1.5*text_height, 0.05, text_height)
				scroll_down=GUI.GUIButton(room,'Scroll Down',self.idname+'d',{'*':None},screen_loc,
					clickHandler=lambda b,p:self.picker.viewMove(3), textfontsize=font_size)
				scroll_down.setCaption(' \\/ ')
				self.gui_element.children.append(scroll_down)

			return self.gui_element
		def draw(self):
			DialogBox.Item.draw(self)
	class Text(Item):
		def __init__(self,text,wid=0.):
			self.text=text
			print text
			self.wid=wid
			DialogBox.Item.__init__(self)
		def height(self):
			return text_height*(1+self.text.count("\n"))
		def width(self):
			return self.wid
		def create(self,owner,room,x,y,wid,hei):
			screen_loc=makeRect(x,y,wid,hei)
			self.gui_element=GUI.GUIStaticText(room,self.idname,self.text, screen_loc,
				color=screen_color, fontsize=font_size,
				bgcolor=GUI.GUIColor.clear())
			return self.gui_element
		
	class Button(Item):
		def __init__(self,id,text,sprite,width,height):
			self.text=text
			self.sprite=sprite
			self.wid=width
			self.hei=height
			if not self.hei:
				self.hei=text_height
			DialogBox.Item.__init__(self,id)
		def height(self):
			return self.hei+text_height
		def width(self):
			return self.wid
		def handleButton(self,button,params):
			button.owner.handleButton(self.id)
		def create(self,owner,room,x,y,wid,hei):
			screen_loc=makeRect(x+text_height/4.,y-text_height/2.,wid-text_height/2.,self.hei)
			text=self.text or ''
			self.gui_element=GUI.GUIButton(room,''+self.text,self.idname,{'*':self.sprite},screen_loc,
				clickHandler=self.handleButton, owner=owner, 
				textfontsize=font_size,textbgcolor=GUI.GUIColor(0.3,0.,0.,.5))
			if self.text:
				self.gui_element.setCaption('  '+self.text)
			return self.gui_element
		
	class TextInput(Item):
		def __init__(self,id,text):
			self.text=text
			DialogBox.Item.__init__(self,id)
		def height(self):
			return text_height
		def getValues(self,values):
			values.append(self.id)
			values.append(self.gui_element.getText())
		def handleButton(self,textarea):
			textarea.owner.close(not textarea.canceled)
		def create(self,owner,room,x,y,wid,hei):
			screen_loc=makeRect(x,y,wid,hei)
			self.gui_element=GUI.GUILineEdit(self.handleButton,room,self.idname,self.text, screen_loc,
				color=input_color,fontsize=font_size,bgcolor=screen_bgcolor,focusbutton=True,owner=owner)
			return self.gui_element
		
		
	class Row(Item):
		def __init__(self,items):
			self.items=items
			DialogBox.Item.__init__(self)
		def width(self):
			totalwid=0.
			for el in self.items:
				wid=el.width()
				#if wid==0.:
				#	return 0.
				totalwid+=wid
			return totalwid
		def getValues(self,values):
			for el in self.items:
				el.getValues(values)
		def height(self):
			return reduce(max,map(lambda x:x.height(),self.items))
		def create(self,owner,room,x,y,wid,hei):
			screen_loc=makeRect(x,y,wid,hei)
			self.gui_element=GUI.GUIGroup(room)
			if self.items:
				totalwid=0.
				num_variable=0
				for it in self.items:
					w=it.width()
					if w==0:
						num_variable+=1
					totalwid+=w
				childs=[]
				avgwid = (wid-totalwid) / num_variable
				for it in self.items:
					w=it.width()
					if w==0:
						w=avgwid
					it.create(owner,room,x,y,w,hei)
					x+=w
					if it.gui_element:
						childs.append(it.gui_element)
				self.gui_element.children = childs
			return self.gui_element
		def draw(self):
			for it in self.items:
				it.draw()
			return DialogBox.Item.draw(self)
		def undraw(self):
			for it in self.items:
				it.undraw()
			return DialogBox.Item.undraw(self)
		
	def __init__(self,elements,callback):
		self.elements=elements
		self.bg=None
		self.bglink=None
		self.roomid=0
		self.callback=callback
		DialogBox.Item.next_id+=1
		self.idname='dialog_bg_'+str(DialogBox.Item.next_id)
		self.id=None

	def handleButton(self,id):
		values=[id]
		for el in self.elements:
			el.getValues(values)
		self.callback(self,values)
	
	def calculateWidth(self):
		totalheight=0.
		totalwidth=0.
		for el in self.elements:
			totalheight+=el.height()
			wid=el.width()
			if wid>totalwidth:
				totalwidth=wid
		self.totalheight=totalheight
		self.totalwidth=totalwidth
	
	def create(self,roomid):
		if VS.isserver():
			return
		self.calculateWidth()
		x=self.totalwidth/-2.
		y=self.totalheight/2.
		room=GUI.GUIRoom(roomid)
		self.roomid=roomid
		if not self.bg:
			rect = makeRect(x-0.01,y+0.01,self.totalwidth+0.02,self.totalheight+0.02)
			size=rect.getHotRect()
			self.bgsize=size
			#print str(rect)
			#print (x,y,self.totalwidth,self.totalheight)
			self.bg=GUI.GUIStaticText(room,self.idname,'',rect,
				color=GUI.GUIColor(0.6,0.6,0.6,1.0), bgcolor=GUI.GUIColor(0.2, 0.2, 0.2, 0.8))
			self.bglink=GUI.GUIMouseOver(room,'Dialog box',self.idname,rect)
		for el in self.elements:
			hei=el.height()
			#print "*** Creating "+repr(el)+" at y "+str(y)+", height "+str(hei)
			el.create(self,room,x,y,self.totalwidth,hei)
			y-=hei
	
	def close(self,success=False):
		if success:
			self.handleButton('OK')
		else:
			self.handleButton('Cancel')
	def keyDown(self,key):
		debug.debug("dialog box got key: %i" % key)
		if key == 13 or key == 10: #should be some kind of return
			self.close(True)
		elif key == 27: #escape is always 27, isn't it?
			self.close(False)
	
	def undraw(self):
		if not VS.isserver():
			GUI.GUIRootSingleton.keyTarget=self.lastKeyTarget
			#debug.warn("Targetting keys to "+str(self.lastKeyTarget))
			self.bg.hide()
			self.bg.undraw()
			self.bglink.hide()
			self.bglink.undraw()
			map(lambda x:x.undraw(), self.elements)
	
	def draw(self):
		if not VS.isserver():
			self.lastKeyTarget=GUI.GUIRootSingleton.keyTarget
			GUI.GUIRootSingleton.keyTarget=self
			#debug.warn("Targetting keys to "+str(self))
			self.bg.show()
			self.bg.draw()
			self.bglink.show()
			self.bglink.draw()
			map(lambda x:x.draw(), self.elements)
	

def fromValues(data):
	action = data[0]
	i=1
	inputs={}
	while i<len(data):
		inputs[data[i]] = data[i+1]
		i+=2
	return action, inputs

_gui_has_initted=False

def dialog(args, callback, room=None):
	global _gui_has_initted

	db = DialogBox(parse_dialog_box(args),callback)
	if VS.isserver():
		db.id = custom.run("dialog_box",args,lambda data:callback(db,data))
		return db.id
	if not room:
		room=Base.GetCurRoom()
	if not _gui_has_initted:
		GUI.GUIInit(320,200)
		_gui_has_initted=True
	db.create(room)
	db.draw()
	GUI.GUIRootSingleton.broadcastMessage('draw',None)

def custom_run_dialog(local, cmd, args, id):
	if VS.isserver():
		return
	
	def myCallback(db,data):
		def serverCallback(data):
			if data[0]=='close':
				db.undraw()
		
		custom.respond(data,serverCallback,id)
	dialog(args, myCallback, Base.GetCurRoom())


def button_row(width, *buttons):
	items = ["height",0.05,"row"]
	if len(buttons)==1:
		items += ["width",(width-0.25)/2.,
			"button",buttons[0],
			"width",(width-0.25)/2.]
	elif len(buttons)==2:
		items += ["button",buttons[0],"width",(width-0.5),"button",buttons[1]]
	elif len(buttons)==3:
		items += ["button",buttons[0],"width",(width-0.75)/2.,
			"button",buttons[1],"width",(width-0.75)/2.,
			"button",buttons[2]]
	else:
		for b in buttons:
			items += ["button",b]
	items += ["endrow"]
	return items

def alert(message, callback=None, width=1.0, buttonText='OK'):
	def dbcallback(db,data):
		db.undraw()
		custom.respond(["close"],None,db.id)
		if callback:
			return callback()
	
	dialog(["width",width, "text",message] + button_row(width, buttonText), dbcallback)

def confirm(message, callback, width=1.0, buttons=('Cancel','OK')):
	def dbcallback(db,data):
		db.undraw()
		custom.respond(["close"],None,db.id)
		arg = False
		if data[0]!=buttons[0] and data[0]!="Cancel":
			arg = data[0]
		if callback:
			return callback(arg)
	
	dialog(["width",width, "text",message] + button_row(width, *buttons), dbcallback)


custom.add("dialog_box",custom_run_dialog)


