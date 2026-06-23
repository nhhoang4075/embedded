#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

Screen2Presenter::Screen2Presenter(Screen2View& v)
    : view(v)
{
}

void Screen2Presenter::activate()
{
	view.updateBoard(model->getGame()->grid);
	view.updateScore(model->getScore());
}

void Screen2Presenter::deactivate()
{
}

void Screen2Presenter::boardChanged()
{
	view.updateBoard(model->getGame()->grid);
}

void Screen2Presenter::scoreChanged(uint32_t score)
{
    view.updateScore(score);
}
