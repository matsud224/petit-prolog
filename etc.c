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

Term* pointer_shortcut(Term* ptr){
	while(ptr->tag==TERM_POINTER){
		ptr=ptr->value.pointer;
	}
	return ptr;
}

void vartable_shortcut(VariableTable* vt){
	VariableTable* ptr=vt;
	while(ptr->next!=NULL){
		if(ptr->next->value.tag==TERM_POINTER){
			ptr->next->value.value.pointer=pointer_shortcut(ptr->next->value.value.pointer);
		}
		ptr=ptr->next;
	}

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
				subptr->next=subptr->next->next;
			}else{
				subptr=subptr->next;
			}
		}

		ptr=ptr->next;
	}

}

void vartable_show(VariableTable v1){
	VariableTable* ptr=&v1;

	printf("\n-----------------\n");

	while(ptr->next!=NULL){
		printf("%s = ",ptr->next->variable->name);
		term_show(ptr->next->value);
		//printf("(%d)",ptr->next->value.ref_bound);
		printf("\n");

		ptr=ptr->next;
	}

	printf("-----------------\n");

}

void term_show(Term t){
	switch(t.tag){
	case TERM_INTEGER:
		printf("%d",t.value.integer);
		break;
	case TERM_POINTER:
		/*printf("*");*/ term_show(*t.value.pointer);
		break;
	case TERM_STRUCTURE:
		structure_show(*(t.value.structure));
		break;
	case TERM_UNBOUND:
		printf("<unbound>");
		break;
	case TERM_VARIABLE:
		printf("%s",t.value.variable->name);
		break;
	default:
		printf("<unknown>");
		break;
	}
}

void structure_show(Structure s){
	TermList* ptr=&(s.arguments);

	printf("%s",s.functor->name);
	if(ptr->next==NULL){
		//arity=0
		return;
	}

	printf("(");
	while(ptr->next!=NULL){
		term_show(ptr->next->term);
		ptr=ptr->next;
		if(ptr->next!=NULL){printf(", ");}
	}
	printf(")");
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
	VariableTable* ptr2;
	VariableTable* ptrprev;
	VTStack* prev=NULL;
	while(ptr->next!=NULL){
		prev=ptr;
		ptr=ptr->next;
	}
	/*printf("**bonud check start**\n");
	vartable_show(ptr->vartable);*/
	//pointerの束縛解除チェック
	if(vts->next!=NULL && prev!=NULL){
		ptr2=&(ptr->vartable);
		ptrprev=&(prev->vartable);
		while(ptr2->next!=NULL){
			if(ptr2->next->value.tag==TERM_POINTER && ptr2->next->value.ref_bound==1 && ptrprev->next->value.ref_bound==0){
				ptr2->next->value.value.pointer->tag=TERM_UNBOUND;
			}
			ptr2=ptr2->next;
			ptrprev=ptrprev->next;
		}
	}
	/*printf("**bonud check end**\n");
	vartable_show(ptr->vartable);*/

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

void vtstack_boundcheck_top(VTStack vts){
	VTStack* ptr=&vts;
	VariableTable* tptr;
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}
	vartable_boundcheck(ptr->vartable);

	return;
}

void vartable_boundcheck(VariableTable vt){
	VariableTable* tptr;

	tptr=&vt;
	while(tptr->next!=NULL){
		if(tptr->next->value.tag==TERM_POINTER){
			if(tptr->next->value.value.pointer->tag==TERM_UNBOUND){
				tptr->next->value.ref_bound=0;
			}else{
				tptr->next->value.ref_bound=1;
			}
		}
		tptr=tptr->next;
	}

	return;
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
