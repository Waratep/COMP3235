// declaration and defination of some global variable
QUELIMIT = 50;
array Q[50][4]; 
// string will be dealed as char array
array ALLTYPE[13] = "A234567890JQK";
array ALLOP[4] = "+-*/";
coun = 0;
qNo = 0;
array example1[20];
example1[0]='2';
example1[1]='*';
example1[2]='(';
example1[3]='3';
example1[4]='+';
example1[5]='4';
example1[6]=')';
example1[7]='e';

array example2[20]="(3+4)*(1+1)n"; 

valueOf(c){ // convert a char to an int value
  if (c == 'A')
    return 1;
  if (c == '2')
    return 2;
  if (c == '3')
    return 3;
  if (c == '4')
    return 4;
  if (c == '5')
    return 5;
  if (c == '6')
    return 6;
  if (c == '7')
    return 7;
  if (c == '8')
    return 8;
  if (c == '9')
    return 9;
  if (c == '0')
    return 10;
  if (c == 'J')
    return 11;
  if (c == 'Q')
    return 12;
  if (c == 'K')
    return 13;
}

valToChar(val){
  a=@ALLTYPE[val-1];
  return a;
}

valOp(c){
  if (c=='+')
    return 0;
  if (c=='-')
    return 1;
  if (c=='*')
    return 2;
  if (c=='/')
    return 3;
}

precedence(c){ // compute the precedence of an operator
  if (c=='(' || c==')')
    return 0;
  if (c=='+' || c=='-')
    return 1;
  if (c=='*' || c=='/')
    return 2;
}

push(stack, ptr, content){
  stack[ptr]=content;
  ptr=ptr+1;
  return ptr;
}

pop(stack, ptr){
  ptr=ptr-1;
  stack[ptr]=-99999;
  return ptr;
}

peek(stack, ptr){
  return stack[ptr-1];
}

evaluate(input){
  array oprStack[50];
  array valStack[50];
  oprPtr=0;
  valPtr=0;
  strPtr=0;
  while(input[strPtr] != 'e'){
    c=input[strPtr];
    if (c=='A'|| c=='2'|| c=='3'|| c=='4'|| c=='5'|| c=='6'|| c=='7'|| c=='8'|| c=='9'|| c=='0'|| c=='J'|| c=='Q'|| c=='K'){
      valPtr=push(valStack, valPtr, valueOf(c));
    }
    if (c=='(')
      oprPtr=push(oprStack, oprPtr, c);
    if (c=='+' || c=='-' || c=='*' || c=='/'){
      if (oprPtr==0)
        oprPtr=push(oprStack, oprPtr, c);
      else if (precedence(c) > precedence(peek(oprStack, oprPtr)))
        oprPtr=push(oprStack, oprPtr, c);
      else{
        while(oprPtr > 0 && precedence(c) <= precedence(peek(oprStack, oprPtr))){
          ope1=peek(valStack, valPtr);
          valPtr=pop(valStack, valPtr);
          ope2=peek(valStack, valPtr);
          valPtr=pop(valStack, valPtr);
          op=peek(oprStack, oprPtr);
          oprPtr=pop(oprStack, oprPtr);
          valPtr=push(valStack, valPtr, compute(ope1, ope2, valOp(op)));
        }
        oprPtr=push(oprStack, oprPtr, c);
      }
    }
    if (c==')'){
      while (peek(oprStack, oprPtr)!='('){
        ope1=peek(valStack, valPtr);
        valPtr=pop(valStack, valPtr);
        ope2=peek(valStack, valPtr);
        valPtr=pop(valStack, valPtr);
        op=peek(oprStack, oprPtr);
        oprPtr=pop(oprStack, oprPtr);
        valPtr=push(valStack, valPtr, compute(ope1, ope2, valOp(op)));
      }
      oprPtr=pop(oprStack, oprPtr); 
    }
    strPtr=strPtr+1;
  } 

  while (oprPtr>0){
    ope1=peek(valStack, valPtr);
    valPtr=pop(valStack, valPtr);
    ope2=peek(valStack, valPtr);
    valPtr=pop(valStack, valPtr);
    op=peek(oprStack, oprPtr);
    oprPtr=pop(oprStack, oprPtr);
    valPtr=push(valStack, valPtr, compute(ope1, ope2, valOp(op)));
  }
  res = peek(valStack, valPtr);
  
  return res;
}


compute(x, y, op)
{
  if(op == 0)
  {
    return x + y;
  }
  else if(op == 1)
  {
    return x - y;
  }
  else if(op == 2)
  {
    return x * y;
  }
  else if(op == 3)
  {
    if(y == 0)
    {
      return 0;
    }
    else
    {
      return x / y;
    }
  }
  else
  {
    return 0;
  }

}

cal1(n1, n2, n3, n4, op1, op2, op3)
{
  r1 = compute(n1, n2, op1);
  r2 = compute(r1, n3, op2);
  r3 = compute(r2, n4, op3);
  return r3;
}

cal2(n1, n2, n3, n4, op1, op2, op3)
{
  r1 = compute(n2, n3, op2);
  r2 = compute(n1, r1, op1);
  r3 = compute(r2, n4, op3);
  return r3;
}

cal3(n1, n2, n3, n4, op1, op2, op3)
{
  r1 = compute(n3, n4, op3);
  r2 = compute(n2, r1, op2);
  r3 = compute(n1, r2, op1);
  return r3;
}

cal4(n1, n2, n3, n4, op1, op2, op3)
{
  r1 = compute(n2, n3, op2);
  r2 = compute(r1, n4, op3);
  r3 = compute(n1, r2, op1);
  return r3;
}

cal5(n1, n2, n3, n4, op1, op2, op3)
{
  r1 = compute(n1, n2, op1);
  r2 = compute(n3, n4, op3);
  r3 = compute(r1, r2, op2);
  return r3;
}

getOp(op)
{
  if(op == 0)
  {
    return '+';
  }
  else if(op == 1)
  {
    return '-';
  }
  else if(op == 2)
  {
    return '*';
  }
  else if(op == 3)
  {
    return '/';
  }
  else
  {
    return '';
  }
}

getResult(n1, n2, n3, n4)
{
  for(op1 = 0; op1 < 4; op1 = op1 + 1;)
  {
    for(op2 = 0; op2 < 4; op2 = op2 + 1;)
    {
      for(op3 = 0; op3 < 4; op3 = op3 + 1;)
      {
        if(cal1(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          putc_('(');
          putc_('(');
          puti_(n1);
          putc_(getOp(op1));
          puti_(n2);
          putc_(')');
          putc_(getOp(op2));
          puti_(n3);
          putc_(')');
          putc_(getOp(op3));
          puti(n4);
        }
        if(cal2(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          putc_('(');
          puti_(n1);
          putc_(getOp(op1));
          putc_('(');
          puti_(n2);
          putc_(getOp(op2));
          puti_(n3);
          putc_(')');
          putc_(')');
          putc_(getOp(op3));
          puti(n4);
        }
        if(cal3(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          puti_(n1);
          putc_(getOp(op1));
          putc_('(');
          puti_(n2);
          putc_(getOp(op2));
          putc_('(');
          puti_(n3);
          putc_(getOp(op3));
          puti_(n4);
          putc_(')');
          putc(')');
        }
        if(cal4(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          puti_(n1);
          putc_(getOp(op1));
          putc_('(');
          putc_('(');
          puti_(n2);
          putc_(getOp(op2));
          puti_(n3);
          putc_(')');
          putc_(getOp(op3));
          puti_(n4);
          putc(')');
        }
        if(cal5(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          putc_('(');
          puti_(n1);
          putc_(getOp(op1));
          puti_(n2);
          putc_(')');
          putc_(getOp(op2));
          putc_('(');
          puti_(n3);
          putc_(getOp(op3));
          puti_(n4);
          putc(')');
        }
      }
    }
  }
}


check(n1, n2, n3, n4)
{
  for(op1 = 0; op1 < 4; op1 = op1 + 1;)
  {
    for(op2 = 0; op2 < 4; op2 = op2 + 1;)
    {
      for(op3 = 0; op3 < 4; op3 = op3 + 1;)
      {
        if(cal1(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          return 1;
        }
        if(cal2(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          return 1;
        }
        if(cal3(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          return 1;
        }
        if(cal4(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          return 1;
        }
        if(cal5(n1, n2, n3, n4, op1, op2, op3) == 24)
        {
          return 1;
        }
        return 0;
      }
    }
  }
}

genQ()
{
  count = 0;
  for(n1 = 1; n1 < 14; n1 = n1 + 1;)
  {
    for(n2 = 1; n2 < 14; n2 = n2 + 1;)
    {
      for(n3 = 1; n3 < 14; n3 = n3 + 1;)
      {
        for(n4 = 1; n4 < 14; n4 = n4 + 1;)
        {
          if(n1 == n2 || n2 == n3 || n3 == n4 || n1 == n3 || n1 == n4 || n2 == n4)
          {
            continue;
          }
          if(check(n1, n2, n3, n4) == 1)
          {
            @Q[count][0] = n1;
            @Q[count][1] = n2;
            @Q[count][2] = n3;
            @Q[count][3] = n4;
            count = count + 1;
            if(count >= @QUELIMIT)
            {
              return 0;
            }
            puti_(n1);
            putc_(' ');
            puti_(n2);
            putc_(' ');
            puti_(n3);
            putc_(' ');
            puti(n4);
          }
        }
      }
    }
  }
}

printQ()
{
  for(i = 0; i < @QUELIMIT; i = i + 1;)
  {
    for(j = 0; j < 4; j = j + 1;)
    {
      puti_(@Q[i][j]);
      putc_(' ');
    }
    putc(' ');
  }
}

puti(evaluate(example1));

genQ();
printQ();