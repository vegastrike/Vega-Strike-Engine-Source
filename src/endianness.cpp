union LILdubl {
  double d;
  unsigned int i[2];
};
union LILfloat {
  float f;
  unsigned int i;
};
float NXSwapHostFloatToLittle (float x) {
  LILfloat l;
  l.f = x;
  l.i = le32_to_cpu (l.i);
  return l.f;
}
double NXSwapHostDoubleToLittle (double x) {
  return le64_to_cpu (x);
}
double DONTUSE__NXSwapBigDoubleToLittleEndian(double x){
  LILdubl l;
  l.d =x;
  unsigned int tmp = le32_to_cpu (l.i[0]);
  l.i[0]= le32_to_cpu (l.i[1]);
  l.i[1]=tmp;
  return l.d;
}
