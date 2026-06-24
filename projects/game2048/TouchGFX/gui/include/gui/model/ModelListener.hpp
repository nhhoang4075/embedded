#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include <stdint.h>

class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }

    virtual void boardChanged() {}
    virtual void scoreChanged(uint32_t score) {}
    /* SW joystick (ấn xuống stick). Mỗi Presenter override để điều hướng:
     *   Screen1 -> goto Screen2 (start game)
     *   Screen2 -> goto Screen1 (thoát, giữ điểm trong RAM) */
    virtual void swPressed() {}
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
