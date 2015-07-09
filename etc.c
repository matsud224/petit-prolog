#include <stdio.h>
#include <malloc.h>
#include "header.h"


void error(char* msg){
    fprintf(stderr,"ERROR: %s\n",msg);
    return;
}


void vartable_add(VariableTable *vl,Variable var){
	VariableTable* ptr=vl;
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}
	ptr->next=malloc(sizeof(VariableTable));
	ptr->next->variable=var;
	ptr->next->next=NULL;

	return;
}

void vartable_concat(VariableTable* dest,VariableTable src){
	VariableTable* ptr=dest;
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}
	ptr->next=src.next;
	return;
}

void vartable_unique(VariableTable vl){
	VariableTable* ptr=&vl;
	VariableTable* subptr;
	VariableTable* temp;
	while(ptr->next!=NULL){
		subptr=ptr;
		while(subptr->next!=NULL){
			if(subptr->next->variable==ptr->next->variable){
				temp=subptr->next;
				subptr->next=subptr->next->next;
				free(temp);
			}

			subptr=subptr->next;
		}

		ptr=ptr->next;
	}
}

Term* vartable_find(VariableTable vl,Variable var){
	VariableTable* ptr=&vl;
	while(ptr->next!=NULL){
		if(ptr->next->variable==var){
			return &(ptr->next->value);
		}
		ptr=ptr->next;
	}
	return NULL;
}

void vtstack_pushnew(VTStack *vts){
	VTStack* ptr=vts;
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}
	ptr->next=malloc(sizeof(VTStack));
	ptr->next->vartable.next=NULL;
	ptr->next->next=NULL;

	return;
}

void vtstack_push(VTStack *vts,VariableTable vartable){
	VTStack* ptr=vts;
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}
	ptr->next=malloc(sizeof(VTStack));
	ptr->next->vartable=vartable;

	return;
}

void vtstack_pop(VTStack *vts){
	VTStack* ptr=vts;
	VTStack* prev=NULL;
	while(ptr->next!=NULL){
		prev=ptr;
		ptr=ptr->next;
	}
	if(prev!=NULL){
		free(prev->next);
	}

	return;
}

VariableTable* vtstack_toptable(VTStack vts){
	VTStack* ptr=&vts;
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}

	return &(ptr->vartable);
}

int structure_arity(Structure s){
	int arity=0;
	TermList* t_ptr=&(s.arguments);
	while(t_ptr->next!=NULL){
		arity++;
		t_ptr=t_ptr->next;
	}
	return arity;
}
