/**
 * $Id: Main.cc,v 1.6 2006/11/24 13:47:03 mxz Exp mxz $
 */

#include <stdio.h>
#include <qt4/QtGui/qapplication.h>
#include <GL/glut.h>
#include "MainWin.hh"
#include "Except.hh"

int main(int argc, char **argv)
{
  try
  {
    glutInit(&argc, argv);
    QApplication app(argc, argv);
    Z::MainWin *main_win = new Z::MainWin(argc, argv);
    //app.setMainWidget(main_win);
    main_win->show();
    app.exec();
    delete main_win;
  }
  catch(Z::Except &e)
  {
    printf("%s\n", e.what());
    return 1;
  }
}

