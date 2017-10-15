#include "UIWidget.h"
#include "Renderer.h"

const unsigned int fbfactor = 2;

unsigned int UIWidget::elementSize = 25; 
unsigned int UIWidget::elementHeight = 25;
unsigned int UIWidget::elementWidth = 8;
UIWidget * UIWidget::currentWidget = NULL;

void UIWidget::addChild(UIWidget * widget){
	this->childs.push_back(widget);
	widget->setParent(this);
}

void UIWidget::drawChilds(Renderer * renderer){
	unsigned int y = renderer->getScreenHeight() / 2 - (this->childs.size() * UIWidget::elementSize) / 2;
	for (int i = 0;i < this->childs.size();i++) {
		this->childs[i]->draw(renderer,renderer->getScreenWidth()/2,y + i * UIWidget::elementSize);
	}
}

void UIWidget::setSelected(bool b){
	this->selected = b;
}

bool UIWidget::isSelected(){
	return this->selected;
}

void UIWidget::setActive(bool b){
	this->active = b;
}

bool UIWidget::isActive(){
	return this->active;
}

void UIWidget::setParent(UIWidget * widget) {
	this->parent = widget;
}

UIWidget * UIWidget::getParent() {
	return this->parent;
}

void UIWidget::handleEvent(SDL_Event event) {
	unsigned int mousex;
	unsigned int mousey;
	static bool yBackDeadzone = true;
	static int lastInputTick = 0;
	int selected = 0;

	if (this->childs.size() == 0) {
		return;
	}

	for (int i = 0;i < this->childs.size();i++) {
		if (this->childs[i]->isSelected()) {
			selected = i;
		}
	}

	switch (event.type)  {
		case SDL_KEYDOWN:
			switch( event.key.keysym.sym )
			{
				case SDLK_UP:
					this->childs[selected]->setSelected(false);
					(--selected < 0) ? selected = selected + this->childs.size() : selected;
					this->childs[selected]->setSelected(true);
					break;
				case SDLK_DOWN:
					this->childs[selected]->setSelected(false);
					selected = (++selected)%this->childs.size();
					this->childs[selected]->setSelected(true);
					break;
				case SDLK_LEFT:
					this->childs[selected]->handleEvent(event);
					break;
				case SDLK_RIGHT:
					this->childs[selected]->handleEvent(event);
					break;
				case SDLK_ESCAPE :
					SDL_SetRelativeMouseMode(SDL_TRUE);
					this->setActive(false);
					break;
				default:
					break;
			}
		break;	
		case SDL_KEYUP:
			switch( event.key.keysym.sym )
			{
				case SDLK_SPACE:
				case SDLK_RETURN:
					this->childs[selected]->onClick();
					break;
				
			}
			break;
		case SDL_MOUSEMOTION:
		case SDL_FINGERMOTION:
			mousex = event.motion.x * fbfactor;
			mousey = event.motion.y;
			for (int i = 0;i < this->childs.size();i++) {
				unsigned int bx,by,tx,ty;
				bx = this->childs[i]->getX();
				by = this->childs[i]->getY();
				tx = bx + this->childs[i]->getLength();
				ty = by + UIWidget::elementSize;
				if (bx <= mousex && tx >= mousex && by <= mousey && ty >= mousey) {
					this->childs[selected]->setSelected(false);
					selected = i;
					this->childs[selected]->setSelected(true);
				}
			}
			break;
		case SDL_JOYBUTTONDOWN:
			if (event.jbutton.button == 4) {
				SDL_SetRelativeMouseMode(SDL_TRUE);
				this->setActive(false);
				break;
			}
		case SDL_FINGERDOWN:
		case SDL_MOUSEBUTTONDOWN:
			this->childs[selected]->onClick();
			break;
		case SDL_JOYAXISMOTION:
			if (event.jaxis.axis == 1) {
				if ((abs(event.jaxis.value) < 400*2)||(SDL_GetTicks() - lastInputTick > 500)) {
					yBackDeadzone = true;
				}

				if (yBackDeadzone && (event.jaxis.value/10 < -400*2)) {
					this->childs[selected]->setSelected(false);
					(--selected < 0) ? selected = selected + this->childs.size() : selected;
					this->childs[selected]->setSelected(true);
					yBackDeadzone = false;	
					lastInputTick = SDL_GetTicks();
				}

				if (yBackDeadzone && (event.jaxis.value/10 > 400*2)) {
					this->childs[selected]->setSelected(false);
					selected = (++selected)%this->childs.size();
					this->childs[selected]->setSelected(true);
					yBackDeadzone = false;
					lastInputTick = SDL_GetTicks();
				}
			}
			if (event.jaxis.axis == 0) {
				this->childs[selected]->handleEvent(event);
			}
			break;
	}
}

void UIWidget::setOnClickCallback(uiwidget_func_cb * func, void * data){
	this->clickFuncCb = func;
	this->clickFuncCbData = data;
}

void UIWidget::onClick() {
	if (this->clickFuncCb != NULL) {
		(*this->clickFuncCb)(this->clickFuncCbData);
	}
}

void UIHeader::setLabel(const char * text){
	this->text.assign(text);
}

void UIHeader::onClick() {
	UIWidget::onClick();
	if (this->childs.size() > 0) {
		UIWidget::currentWidget = this;
	}
}

UIHeader::UIHeader() : UIWidget(){
}

void UIHeader::draw(Renderer * renderer,Uint32 x,Uint32 y){
	std::string s;
	if (selected) {
		s = ">"+this->text;
	} else {
		s = this->text;
	}

	renderer->drawMessage(s.c_str(),x,y);
	this->x = x;
	this->y = y;
	this->length = s.length() * UIWidget::elementWidth;
}

void UIBoolean::setState(bool b) {
	if (b != this->state) {
		this->boolChangeFuncCb(b,this->boolChangeFuncData);
	}
	this->state = b;
}

bool UIBoolean::getState() {
	return this->state;
}
void UIBoolean::draw(Renderer * renderer,Uint32 x,Uint32 y) {
	std::string s;
	if (selected) {
		s = ">"+this->text;
	} else {
		s = this->text;
	}

	if (state) {
		s += " on";
	} else {
		s += " off";
	}

	renderer->drawMessage(s.c_str(),x,y);
	this->x = x;
	this->y = y;
	this->length = s.length() * UIWidget::elementWidth;
}

void UIBoolean::onClick() {
	UIHeader::onClick();
	setState(!this->state);
}

void UIBoolean::setOnBoolChangeCallback(uiwidget_func_bool_change_cb * func, void * data){
	this->boolChangeFuncCb = func;
	this->boolChangeFuncData = data;
}

void func_default_value_increase_cb(void * data) {
	UINumeric * numeric = (UINumeric *)data;
	if ((numeric->getStep() + numeric->getValue()) <= numeric->getMaxValue()) {
		numeric->setValue(numeric->getStep() + numeric->getValue());
	} else {
		numeric->setValue(numeric->getMaxValue());
	}
}

void func_default_value_decrease_cb(void * data) {
	UINumeric * numeric = (UINumeric *)data;
	if (numeric->getValue() - numeric->getStep() >= numeric->getMinValue()) {
		numeric->setValue(numeric->getValue() - numeric->getStep());
	} else {
		numeric->setValue(numeric->getMinValue());
	}
}

UINumeric::UINumeric() : UIHeader () {
	this->setOnValueIncreaseCallback(func_default_value_increase_cb,this);
	this->setOnValueDecreaseCallback(func_default_value_decrease_cb,this);
	this->valueChangeFuncCb = NULL;
	this->valueChangeFuncData = NULL;
};

void UINumeric::setValue(float value) {
	this->value = value;
}

float UINumeric::getValue() {
	return this->value;
}

void UINumeric::setStep(float step) {
	this->step = step;
}

float UINumeric::getStep() {
	return this->step;
}

void UINumeric::setMaxValue(float max) {
	this->max = max;
}

void UINumeric::setMinValue(float min) {
	this->min = min;
}

float UINumeric::getMaxValue() {
	return this->max;
}

float UINumeric::getMinValue() {
	return this->min;
}

void UINumeric::setOnValueIncreaseCallback(uiwidget_func_cb * func, void * data) {
	this->valueIncreaseFuncCb = func;
	this->valueIncreaseFuncData = data;
}

void UINumeric::setOnValueDecreaseCallback(uiwidget_func_cb * func, void * data) {
	this->valueDecreaseFuncCb = func;
	this->valueDecreaseFuncData = data;
}

void UINumeric::setOnValueChangeCallback(uiwidget_func_value_change_cb * func, void * data) {
	this->valueChangeFuncCb = func;
	this->valueChangeFuncData = data;
}

void UINumeric::draw(Renderer * renderer,Uint32 x,Uint32 y) {
	std::string s;
	if (selected) {
		s = ">"+this->text;
	} else {
		s = this->text;
	}

	char sValue[20];
	sprintf(sValue," < %.2f >",value);
	s.append(sValue);

	renderer->drawMessage(s.c_str(),x,y);
	this->x = x;
	this->y = y;
	this->length = s.length() * UIWidget::elementWidth;
}

void UINumeric::handleEvent(SDL_Event event) {
	static bool xBackDeadzone = true;
	static int lastInputTick = 0;
	switch (event.type)  {
		case SDL_KEYDOWN:
			switch( event.key.keysym.sym ) {
				case SDLK_LEFT:
					this->valueDecreaseFuncCb(this->valueDecreaseFuncData);
					if (this->valueChangeFuncCb != NULL) {
						this->valueChangeFuncCb(this->getValue(),this->valueChangeFuncData);
					}
					break;
				case SDLK_RIGHT:
					this->valueIncreaseFuncCb(this->valueIncreaseFuncData);
					if (this->valueChangeFuncCb != NULL) {
						this->valueChangeFuncCb(this->getValue(),this->valueChangeFuncData);
					}
					break;
			}
		break;
		case SDL_JOYAXISMOTION:
			if (event.jaxis.axis == 0) {
				if ((abs(event.jaxis.value) < 400*2) ||(SDL_GetTicks() - lastInputTick > 500)) {
					xBackDeadzone = true;
				} 

				if (xBackDeadzone && (event.jaxis.value/10 < -400*2)) {
					this->valueDecreaseFuncCb(this->valueDecreaseFuncData);
					if (this->valueChangeFuncCb != NULL) {
						this->valueChangeFuncCb(this->getValue(),this->valueChangeFuncData);
					}					
					xBackDeadzone = false;
					lastInputTick = SDL_GetTicks();
				}

				if (xBackDeadzone && (event.jaxis.value/10 > 400*2)) {
					this->valueIncreaseFuncCb(this->valueIncreaseFuncData);
					if (this->valueChangeFuncCb != NULL) {
						this->valueChangeFuncCb(this->getValue(),this->valueChangeFuncData);
					}
					xBackDeadzone = false;
					lastInputTick = SDL_GetTicks();
				}
			}

			break;
	}
}