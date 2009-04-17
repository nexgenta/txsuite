/*
 * cloneobj.h
 */

#ifndef __CLONEOBJ_H__
#define __CLONEOBJ_H__

#define CLONE_OBJECT(OBJ, TYPE, GINAME)											\
do															\
{															\
	RootClass *group;												\
	unsigned int obj_num;												\
	LIST_TYPE(GroupItem) *clone;											\
	ObjectReference ref;												\
	VariableClass *var;												\
															\
	/* find the GroupClass (ie the Application or Scene) that contains this object */				\
	group = MHEGEngine_findGroupObject(&((OBJ)->rootClass.inst.ref.group_identifier));				\
															\
	/* get an unused object number for the clone */									\
	if((obj_num = MHEGEngine_getUnusedObjectNumber(group)) == 0)							\
	{														\
		error("Unable to clone object %s", ExternalReference_name(&((OBJ)->rootClass.inst.ref)));		\
		return;													\
	}														\
															\
	/* create the clone */												\
	clone = safe_malloc(sizeof(LIST_TYPE(GroupItem)));								\
	bzero(clone, sizeof(LIST_TYPE(GroupItem)));									\
	clone->item.choice = GroupItem_ ## GINAME;									\
	TYPE ## _dup(&clone->item.u.GINAME, OBJ);									\
															\
	/* add the clone to our group and register it with the MHEG engine */						\
	RootClass_registerClonedObject(clone, group, obj_num, RTTI_ ## TYPE);						\
															\
	/* do Preparation - also copies the instance vars from the original values */					\
	TYPE ## _Preparation(&clone->item.u.GINAME);									\
															\
	/* store the new ObjectReference in the output variable */							\
	if((var = (VariableClass *) MHEGEngine_findObjectReference(&params->clone_ref_var, caller_gid)) == NULL)	\
		return;													\
															\
	if(var->rootClass.inst.rtti != RTTI_VariableClass								\
	|| VariableClass_type(var) != OriginalValue_object_reference)							\
	{														\
		error("Clone: type mismatch");										\
		return;													\
	}														\
															\
	/* make an ObjectReference from the clone's ExternalReference */						\
	ref.choice = ObjectReference_external_reference;								\
	ref.u.external_reference.group_identifier.data = group->inst.ref.group_identifier.data;				\
	ref.u.external_reference.group_identifier.size = group->inst.ref.group_identifier.size;				\
	ref.u.external_reference.object_number = obj_num;								\
															\
	ObjectRefVariableClass_setObjectReference(var, &ref);								\
}															\
while(0)

#endif	/* __CLONEOBJ_H__ */

