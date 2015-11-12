parent(namihei,sazae).
parent(fune,sazae).
parent(namihei,katsuo).
parent(fune,katsuo).
parent(namihei,wakame).
parent(fune,wakame).
parent(masuo,tarao).
parent(sazae,tarao).
 
couple(namihei,fune).
couple(masuo,sazae).

grandchild(X,Y):-parent(Y,Z),parent(Z,X).
grandparent(X,Y):-grandchild(Y,X).
brother(X,Y):-parent(Z,X),parent(Z,Y).


thief(john).
likes(mary,food).
likes(mary,wine).
likes(lucy,wine).
likes(john,X):-likes(X,wine).
maysteal(X,Y) :- thief(X), likes(X,Y).
?-maysteal(john,X).


owns(john,book(wutheringheights,author(emily,bronte))).
?-owns(john,book(X,author(Y,bronte))).
?-owns(john,book(_,author(_,bronte))).

eq(X,X).
?-eq([A,B,C],[[1,2],[3,4],[5,6,a]]).

append([], Xs, Xs).
append([X | Ls], Ys, [X | Zs]) :- append(Ls, Ys, Zs).
?-append([1],[2],X).

member(X, [X|_]).
member(X, [_|Y]) :- member(X, Y).
?-member(1,[3,a,b,A,5,2,1,2]).

subset([],_).
subset([A|R],B):-member(A,B),subset(R,B).
?-subset([1,2],[3,5,X,5,Y,9]).
?-subset([1,2],[4,3,2,1]).

a(1).
a(2).
a(_).
?-a(X).

b(1):-!.
b(2):-!.
b(_).
?-b(X).

eq(X,X).
?-eq(P,Q),eq(R,S),eq(Q,S),eq(P,2).


different(red,green). different(red,blue).
different(green,red). different(green,blue).
different(blue,red). different(blue,green).
coloring(Hyogo,Kyoto,Shiga,Nara,Osaka,Wakayama) :- different(Hyogo,Kyoto),different(Hyogo,Osaka),different(Kyoto,Shiga),different(Kyoto,Osaka),different(Kyoto,Nara),different(Nara,Osaka),different(Nara,Wakayama),different(Osaka,Wakayama).
?-coloring(Hyogo,Kyoto,Shiga,Nara,Osaka,Wakayama).

