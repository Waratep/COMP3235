array a[1][2][3];
a[0][1][2] = 2;
puti(a[0][1][2]);
aaa(array x[1][2][3])
{
  x[0][1][2] = 111;
  puti(x[0][1][2]);
  bbb(x);
  puti(x[0][1][2]);
}

bbb(array y[1][2][3])
{
  y[0][1][2] = 222;
}

aaa(a);
puti(a[0][1][2]);
