#pragma once

#include <vector>
#include <string>
#include <SDL.h>

class Renderer;

typedef void (uiwidget_func_cb)(void * data);
typedef void (uiwidget_func_bool_change_cb)(bool newState, void * data);
typedef void (uiwidget_func_value_change_cb)(float newValue, void * data);
class UIWidget {

protected :
	static unsigned int elementSize;
	static unsigned int elementWidth;
	static unsigned int elementHeight;
	bool selected;
	bool active;
	std::vector<UIWidget *> childs;
	UIWidget * parent;
	uiwidget_func_cb * clickFuncCb;
	void * clickFuncCbData;
	unsigned int x;
	unsigned int y;
	unsigned int length;
public :
	UIWidget() {
		selected = false;
		active = true;
		clickFuncCb = NULL;
		parent = NULL;
		x = 0;
		y = 0;
	}
	virtual void addChild(UIWidget * widget);
	void setParent(UIWidget * widget);
	UIWidget * getParent();
	void drawChilds(Renderer * renderer);
	void setSelected(bool b);
	bool isSelected();
	void setActive(bool b);
	bool isActive();
	unsigned int getX() { return x; };
	unsigned int getY() { return y; };
	unsigned int getLength() { return length; };
	virtual void draw(Renderer * renderer,Uint32 x,Uint32 y) = 0;
	virtual void handleEvent(SDL_Event event);
	static UIWidget * currentWidget;
	virtual void onClick();
	void setOnClickCallback(uiwidget_func_cb * func, void * data);
};

class UIHeader : public UIWidget {
protected :
	std::string text;	
public :
	UIHeader();
	virtual void setLabel(const char * text);
	virtual void draw(Renderer * renderer,Uint32 x,Uint32 y);
	virtual void onClick();
};

class UIBoolean : public UIHeader {
protected :
	bool state;
	uiwidget_func_bool_change_cb * boolChangeFuncCb;
	void * boolChangeFuncData;
public :
	UIBoolean():UIHeader(){};
	void setState(bool b);
	bool getState();
	void setOnBoolChangeCallback(uiwidget_func_bool_change_cb * func, void * data);
	virtual void draw(Renderer * renderer,Uint32 x,Uint32 y);
	virtual void onClick();
};

class UINumeric : public UIHeader {
protected :
	float value;
	float step;
	float min;
	float max;
	uiwidget_func_cb * valueIncreaseFuncCb;
	void * valueIncreaseFuncData;
	uiwidget_func_cb * valueDecreaseFuncCb;
	void * valueDecreaseFuncData;
	uiwidget_func_value_change_cb * valueChangeFuncCb;
	void * valueChangeFuncData;
public :
	UINumeric();
	void setValue(float value);
	float getValue();
	void setStep(float step);
	float getStep();
	void setMaxValue(float max);
	float getMaxValue();
	void setMinValue(float min);
	float getMinValue();
	void setOnValueIncreaseCallback(uiwidget_func_cb * func, void * data);
	void setOnValueDecreaseCallback(uiwidget_func_cb * func, void * data);
	void setOnValueChangeCallback(uiwidget_func_value_change_cb * func, void * data);
	virtual void draw(Renderer * renderer,Uint32 x,Uint32 y);
	virtual void handleEvent(SDL_Event event);
};