/****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2005                                                \/)\/    *
 * Visual Computing Lab                                            /\/|      *
 * ISTI - Italian National Research Council                           |      *
 *                                                                    \      *
 * All rights reserved.                                                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *   
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
 * for more details.                                                         *
 *                                                                           *
 ****************************************************************************/
/****************************************************************************
  History
$Log: meshedit.cpp,v $
****************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <meshlab/glarea.h>
#include "CADtexturingedit.h"
//#include <wrap/gl/pick.h>
#include<wrap/qt/gl_label.h>
#include<wrap/gl/shot.h>
#include <wrap/glw/glw.h>

using namespace std;
using namespace vcg;
using namespace glw;

CADtexturingEditPlugin::CADtexturingEditPlugin() 
{
	//qFont.setFamily("Helvetica");
	//qFont.setPixelSize(12);

	//haveToPick = false;
	//pickmode = 0; // 0 face 1 vertex
	//curFacePtr = 0;
	//curVertPtr = 0;
	//pIndex = 0;
}

const QString CADtexturingEditPlugin::Info() 
{
	return tr("Return detailed info about a picked face or vertex of the model.");
}
 
void CADtexturingEditPlugin::mouseReleaseEvent(QMouseEvent * event, MeshModel &/*m*/, GLArea * gla)
{
	gla->update();
	//cur = event->pos();
	//haveToPick = true;
	//curFacePtr = 0;
	//curVertPtr = 0;
	//pIndex = 0;
}
void CADtexturingEditPlugin::renderEdges(GLArea * gla){
	int winsize = 512;
	int width = winsize;
	int height = winsize;


	//glarea = gla;
	if (gla->mvc() == NULL)
		return;
	MLSceneGLSharedDataContext* shared = gla->mvc()->sharedDataContext();
	if (shared == NULL)
		return;

	Context ctx;
	ctx.acquire();

	RenderbufferHandle hDepth = createRenderbuffer(ctx, GL_DEPTH_COMPONENT24, width, height);
	Texture2DHandle    hColor = createTexture2D(ctx, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE);
	FramebufferHandle  hFramebuffer = createFramebuffer(ctx, renderbufferTarget(hDepth), texture2DTarget(hColor));

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glViewport(0, 0, winsize, winsize);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, 1.0, 0.1, 100); // SISTEMARE NEAR E FAR
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	ctx.bindReadDrawFramebuffer(hFramebuffer);
	GLW_CHECK_GL_READ_DRAW_FRAMEBUFFER_STATUS;


	MLSceneGLSharedDataContext::PerMeshRenderingDataMap dt;
	shared->getRenderInfoPerMeshView(gla->context(), dt);

	for (int i = 0; i < meshmodel->cm.vert.size(); ++i){
		CMeshO::CoordType p = meshmodel->cm.vert[i].cP();

		QImage image(int(width), int(height), QImage::Format_ARGB32);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, winsize, winsize);
		glPushMatrix();

		/* set camera*/
		GlShot<Shotm>::SetView(gla->mvc()->meshDoc.rm()->shot, 1, 100);
		
		/**/

		{
			MLSceneGLSharedDataContext* datacont = gla->mvc()->sharedDataContext();
			if (datacont == NULL)
				return;

			foreach(MeshModel * mp, gla->md()->meshList){
				MLRenderingData curr;
				datacont->getRenderInfoPerMeshView(mp->id(), gla->context(), curr);
				MLPerViewGLOptions opts;
				curr.get(opts);
				//if (curr.get(opts) == false)
				//	throw MLException(QString("GLArea: invalid MLPerViewGLOptions"));
				//gla->setLightingColors(opts);


				if (opts._back_face_cull)
					glEnable(GL_CULL_FACE);
				else
					glDisable(GL_CULL_FACE);

				datacont->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);

				if (mp->cm.fn)
					datacont->draw(mp->id(), gla->context());
			}
		}

		glReadPixels(0, 0, 3 * winsize, 2 * winsize, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

		image.rgbSwapped().mirrored().save("pano_" + QString().setNum(i) + ".jpg");
	}
	ctx.unbindReadDrawFramebuffer();
	ctx.release();
	glViewport(vp[0], vp[1], vp[2], vp[3]);
	return;

}
void CADtexturingEditPlugin::on_renderEdges(){
	drawEdgesTrigger = true;
	glarea->update();
}
void CADtexturingEditPlugin::Decorate(MeshModel &m, GLArea * gla, QPainter *p)
{
	if (drawEdgesTrigger){
		renderEdges(gla);
		drawEdgesTrigger = false;
	}
}
void CADtexturingEditPlugin::keyReleaseEvent(QKeyEvent *e, MeshModel &m, GLArea *gla)
{
	//if (e->key() == Qt::Key_Space) // toggle pick mode
	//{
	//	pickmode = (pickmode + 1) % 2;
	//	curFacePtr = 0;
	//	curVertPtr = 0;
	//	pIndex = 0;
	//	gla->update();
	//}

	//if ((e->key() == Qt::Key_Q) && ((curFacePtr != 0) || (curVertPtr != 0))) // tab to next selected mode
	//{
	//	if (pickmode == 0)
	//	{
	//		pIndex = (pIndex + 1) % NewFaceSel.size();
	//		curFacePtr = NewFaceSel[pIndex];
	//	}
	//	else if (pickmode == 1)
	//	{
	//		pIndex = (pIndex + 1) % NewVertSel.size();
	//		curVertPtr = NewVertSel[pIndex];
	//	}
	//	gla->update();
	//}

	//if ((e->key() == Qt::Key_P) && ((curFacePtr != 0) || (curVertPtr != 0))) // tab to next selected mode
	//{
	//	// print on log
	//	if (pickmode == 0)
	//	{
	//		this->Log(GLLogStream::FILTER, "------");
	//		this->Log(GLLogStream::FILTER, "face# %i : vert# (%i %i %i)", tri::Index(m.cm, curFacePtr), tri::Index(m.cm, curFacePtr->V(0)), tri::Index(m.cm, curFacePtr->V(1)), tri::Index(m.cm, curFacePtr->V(2)));
	//		if (m.hasDataMask(MeshModel::MM_FACECOLOR))
	//			this->Log(GLLogStream::FILTER, "face color (%i %i %i %i)", curFacePtr->C()[0], curFacePtr->C()[1], curFacePtr->C()[2], curFacePtr->C()[3]);
	//		for (int i = 0; i < 3; ++i)
	//		{
	//			this->Log(GLLogStream::FILTER, "face vert %i : vert# %i", i, tri::Index(m.cm, curFacePtr->V(i)));
	//			this->Log(GLLogStream::FILTER, "position [%f %f %f]", curFacePtr->V(i)->P()[0], curFacePtr->V(i)->P()[1], curFacePtr->V(i)->P()[2]);
	//			this->Log(GLLogStream::FILTER, "normal [%f %f %f]", curFacePtr->V(i)->N()[0], curFacePtr->V(i)->N()[1], curFacePtr->V(i)->N()[2]);
	//			if (m.hasDataMask(MeshModel::MM_VERTQUALITY))
	//				this->Log(GLLogStream::FILTER, "quality %f", curFacePtr->V(i)->Q());
	//			if (m.hasDataMask(MeshModel::MM_VERTCOLOR))
	//				this->Log(GLLogStream::FILTER, "color (%f %f %f %f)", curFacePtr->V(i)->C()[0], curFacePtr->V(i)->C()[1], curFacePtr->V(i)->C()[2], curFacePtr->V(i)->C()[3]);
	//			if (m.hasDataMask(MeshModel::MM_WEDGTEXCOORD))
	//				this->Log(GLLogStream::FILTER, "wedge UV (%f %f) texID %i)", curFacePtr->WT(i).U(), curFacePtr->WT(i).V(), curFacePtr->WT(i).N());
	//			if (m.hasDataMask(MeshModel::MM_VERTTEXCOORD))
	//				this->Log(GLLogStream::FILTER, "vertex UV (%f %f) texID %i)", curFacePtr->V(i)->T().U(), curFacePtr->V(i)->T().U(), curFacePtr->V(i)->T().N());
	//		}
	//		this->Log(GLLogStream::FILTER, "------");
	//	}
	//	else if (pickmode == 1)
	//	{
	//		this->Log(GLLogStream::FILTER, "------");
	//		this->Log(GLLogStream::FILTER, "vertex# %i", tri::Index(m.cm, curVertPtr));
	//		this->Log(GLLogStream::FILTER, "position [%f %f %f]", curVertPtr->P()[0], curVertPtr->P()[1], curVertPtr->P()[2]);
	//		this->Log(GLLogStream::FILTER, "normal [%f %f %f]", curVertPtr->N()[0], curVertPtr->N()[1], curVertPtr->N()[2]);
	//		if (m.hasDataMask(MeshModel::MM_VERTQUALITY))
	//			this->Log(GLLogStream::FILTER, "quality %f", curVertPtr->Q());
	//		if (m.hasDataMask(MeshModel::MM_VERTCOLOR))
	//			this->Log(GLLogStream::FILTER, "color (%f %f %f %f)", curVertPtr->C()[0], curVertPtr->C()[1], curVertPtr->C()[2], curVertPtr->C()[3]);
	//		if (m.hasDataMask(MeshModel::MM_VERTTEXCOORD))
	//			this->Log(GLLogStream::FILTER, "vertex UV (%f %f) texID %i)", curVertPtr->T().U(), curVertPtr->T().U(), curVertPtr->T().N());
	//		this->Log(GLLogStream::FILTER, "------");
	//	}
	//	gla->update();
	//}
}



bool CADtexturingEditPlugin::StartEdit(MeshModel & m , GLArea * gla, MLSceneGLSharedDataContext* /*cont*/)
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
		return false;

	gla->setCursor(QCursor(QPixmap(":/images/cur_info.png"),1,1));	
	connect(this, SIGNAL(suspendEditToggle()), gla, SLOT(suspendEditToggle()));

	dock = new QDockWidget(gla->window());
	control = new CADtexturingControl(dock);
	meshmodel = &m;
	glarea = gla;
	dock->setAllowedAreas(Qt::NoDockWidgetArea);
	dock->setWidget(control);
	QPoint p = gla->mapToGlobal(QPoint(0, 0));
	dock->setGeometry(5 + p.x(), p.y() + 5, control->width(), control->height() );
	dock->setFloating(true);
	dock->setVisible(true);

	QObject::connect(control, SIGNAL(renderEdgesClicked()), this, SLOT(on_renderEdges()));
	drawEdgesTrigger = false;
	return true;
}
