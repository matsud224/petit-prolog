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
	ptr->next->value.tag=TERM_UNBOUND;
	ptr->next->next=NULL;

	return;
}

VariableTable vartable_copy(VariableTable vl){
	VariableTable newtable; newtable.next=NULL;
	VariableTable* ptr=&vl;
	VariableTable* n_ptr=&newtable;
	while(ptr->next!=NULL){
		n_ptr->next=malloc(sizeof(VariableTable));
		n_ptr->next->next=NULL;
		n_ptr->next->variable=ptr->next->variable;
		n_ptr->next->value=ptr->next->value;
		ptr=ptr->next;
		n_ptr=n_ptr->next;
	}

	return newtable;
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
		subptr=ptr->next;
		while(subptr->next!=NULL){
			if(subptr->next->variable==ptr->next->variable){
				temp=subptr->next;
				subptr->next=subptr->next->next;
				subptr=temp;
			}else{
				subptr=subptr->next;
			}
		}

		ptr=ptr->next;
	}

}

void vartable_show(VariableTable v1){
	VariableTable* ptr=&v1;

	printf("---variable(s)---\n",ptr->next->variable->name);

	while(ptr->next!=NULL){
		printf("%s = ",ptr->next->variable->name);
		term_show(ptr->next->value);
		printf("\n");

		ptr=ptr->next;
	}

	printf("-----------------\n");

}

Term* vartable_find(VariableTable vl,Variable var){
	VariableTable* ptr=&vl;

	while(ptr->next!=NULL){
		if(ptr->next->variable==var){
			return &(ptr->next->value);
		}
		ptr=ptr->next;
	}
	printf("variable: not found! (%s)\n",var->name);

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
	ptr->next->next=NULL;
	return;
}

void vtstack_duplicate(VTStack *vts){
	vtstack_push(vts,vartable_copy(*vtstack_toptable(*vts)));

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
		prev->next=NULL;
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

int vtstack_size(VTStack vts){
	VTStack* ptr=&vts;
	int size=0;
	while(ptr->next!=NULL){
		size++;
		ptr=ptr->next;
	}

	return size;
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
