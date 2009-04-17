/*
 * MHEGApp.c
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "MHEGEngine.h"
#include "MHEGApp.h"
#include "ApplicationClass.h"
#include "utils.h"

void
MHEGApp_init(MHEGApp *m)
{
	m->app = NULL;
	m->scene = NULL;

	return;
}

void
MHEGApp_fini(MHEGApp *m)
{
	if(m->app != NULL)
		free_InterchangedObject(m->app);

	if(m->scene != NULL)
		free_InterchangedObject(m->scene);

	safe_free(m->app);
	safe_free(m->scene);

	/* in case it gets reused */
	MHEGApp_init(m);

	return;
}

/*
 * derfile should be an absolute group ID, ie start with ~//
 * frees any exisiting application
 * returns NULL if it can't load it
 */

ApplicationClass *
MHEGApp_loadApplication(MHEGApp *m, OctetString *derfile)
{
	FILE *der;
	int len;
	int rc;

	/* assert */
	if(derfile->size < 3 || strncmp(derfile->data, "~//", 3) != 0)
		fatal("MHEGApp_loadApplication: group ID '%.*s' is not absolute", derfile->size, derfile->data);

	if(m->app != NULL)
		free_InterchangedObject(m->app);
	else
		m->app = safe_malloc(sizeof(InterchangedObject));
	bzero(m->app, sizeof(InterchangedObject));

	if((der = MHEGEngine_openFile(derfile)) == NULL)
	{
		error("Unable to open '%.*s'", derfile->size, derfile->data);
		safe_free(m->app);
		m->app = NULL;
		return NULL;
	}

	/* so all the ObjectReferences get resolved to the current file */
	MHEGEngine_setDERObject(derfile);
	/* DER decode it */
	fseek(der, 0, SEEK_END);
	len = ftell(der);
	rewind(der);
	rc = der_decode_InterchangedObject(der, m->app, len);
	fclose(der);

	if(rc < 0 || m->app->choice != InterchangedObject_application)
	{
		free_InterchangedObject(m->app);
		safe_free(m->app);
		m->app = NULL;
		if(rc < 0)
			error("Unable to load '%.*s'", derfile->size, derfile->data);
		else
			error("No ApplicationClass in '%.*s'", derfile->size, derfile->data);
		return NULL;
	}

	return &m->app->u.application;
}

/*
 * derfile should be an absolute group ID, ie start with ~//
 * frees the existing active scene if there is one
 * returns NULL if it can't load the new one
 */

SceneClass *
MHEGApp_loadScene(MHEGApp *m, OctetString *derfile)
{
	FILE *der;
	int len;
	int rc;

	/* assert */
	if(derfile->size < 3 || strncmp(derfile->data, "~//", 3) != 0)
		fatal("MHEGApp_loadApplication: group ID '%.*s' is not absolute", derfile->size, derfile->data);

	if(m->scene != NULL)
		free_InterchangedObject(m->scene);
	else
		m->scene = safe_malloc(sizeof(InterchangedObject));
	bzero(m->scene, sizeof(InterchangedObject));

	if((der = MHEGEngine_openFile(derfile)) == NULL)
	{
		error("Unable to open '%.*s'", derfile->size, derfile->data);
		safe_free(m->scene);
		m->scene = NULL;
		return NULL;
	}

	/* so all the ObjectReferences get resolved to the current file */
	MHEGEngine_setDERObject(derfile);
	/* DER decode it */
	fseek(der, 0, SEEK_END);
	len = ftell(der);
	rewind(der);
	rc = der_decode_InterchangedObject(der, m->scene, len);
	fclose(der);

	if(rc < 0 || m->scene->choice != InterchangedObject_scene)
	{
		free_InterchangedObject(m->scene);
		safe_free(m->scene);
		m->scene = NULL;
		if(rc < 0)
			error("Unable to load '%.*s'", derfile->size, derfile->data);
		else
			error("No SceneClass in '%.*s'", derfile->size, derfile->data);
		return NULL;
	}

	return &m->scene->u.scene;
}


