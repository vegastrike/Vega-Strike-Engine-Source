
inline void Quaternion::to_matrix(Matrix mat) const {
  float W = s, X = v.i, Y = v.j, Z = v.k;
    
  float xx      = X * X;
  float xy      = X * Y;
  float xz      = X * Z;
  float xw      = X * W;

  float yy      = Y * Y;
  float yz      = Y * Z;
  float yw      = Y * W;

  float zz      = Z * Z;
  float zw      = Z * W;

  mat[0]  = 1 - 2 * ( yy + zz );
  mat[1]  =     2 * ( xy + zw );
  mat[2]  =     2 * ( xz - yw );

  mat[4]  =     2 * ( xy + zw );
  mat[5]  = 1 - 2 * ( xx - zz );
  mat[6]  =     2 * ( yz + xw );

  mat[8]  =     2 * ( xz + yw );
  mat[9]  =     2 * ( yz - xw );
  mat[10] = 1 - 2 * ( xx - yy );

  mat[3]  = mat[7] = mat[11] = mat[12] = mat[13] = mat[14] = 0;
  mat[15] = 1;
}

inline Quaternion Quaternion::from_vectors(const Vector &v1, const Vector &v2, const Vector &v3) {
  float T = v1.i + v2.j + v3.k + 1, S, W, X, Y, Z;
  
  if(T>=0) {
    
    S = 0.5 / sqrt(T);
    
    W = 0.25 / S;
      
    X = ( v2.k - v3.i ) * S;
      
    Y = ( v3.i - v1.k ) * S;
    
    Z = ( v1.j - v2.i ) * S;
  }
  else {
    int max = (v1.i>v2.j)?1:2;
    if(max==1)
      max = (v1.i>v3.k)?1:3;
    else 
      max = (v2.j>v3.k)?2:3;
    switch(max) {
    case 1:
      //column 1
      S  = sqrt( 1.0 + v1.i - v2.j - v3.k ) * 2;
      X = 0.5 / S;
      Y = (v2.i + v1.j ) / S;
      Z = (v1.k + v3.i ) / S;
      W = (v2.k - v3.j ) / S;
      break;
    case 2:
      //column 2
      S  = sqrt( 1.0 + v2.j - v1.i - v3.k ) * 2;
      
      X = (v2.i + v1.j ) / S;
      Y = 0.5 / S;
      Z = (v3.j + v2.k ) / S;
      W = (v3.i - v1.k ) / S;
      break;
    case 3:
      //column 3    
      S  = sqrt( 1.0 + v3.k - v1.i - v2.j ) * 2;
      
      X = (v1.k + v3.i ) / S;
      Y = (v3.j + v2.k ) / S;
      Z = 0.5 / S;
      W = (v1.j - v2.i ) / S;
      break;
    }
  }
  return Quaternion(W, Vector(X,Y,Z));
}

inline Quaternion Quaternion::from_axis_angle(const Vector &axis, float angle) {
  float sin_a = sin( angle / 2 );
  float cos_a = cos( angle / 2 );
  
  return Quaternion(cos_a, Vector(axis.i / sin_a, 
		axis.j / sin_a,
		axis.k / sin_a));
}

