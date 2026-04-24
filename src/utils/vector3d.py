import numpy as np

class Vector3D(np.ndarray):
    def __new__(cls, x, y, z, homogeneous=1.0):
        obj = np.array([x, y, z, homogeneous], dtype=float).view(cls)
        return obj

    def normalize(self):
        length = self.length()
        self.x /= length
        self.y /= length
        self.z /= length

    def length(self):
        return np.linalg.norm(self[:3])

    def cross(self, other):
        return np.cross(self[:3], other[:3])

    def dot(self, other):
        return np.dot(self[:3], other[:3])

    @property
    def x(self):
        return self[0]

    @x.setter
    def x(self, value):
        self[0] = value

    @property
    def y(self):
        return self[1]

    @y.setter
    def y(self, value):
        self[1] = value

    @property
    def z(self):
        return self[2]

    @z.setter
    def z(self, value):
        self[2] = value

    @classmethod
    def vector(cls, x, y, z):
        return cls(x, y, z, 0.0)

    @classmethod
    def point(cls, x, y, z):
        return cls(x, y, z, 1.0)
    
    #Fix multiply/divide behavior
    def __mul__(self, scalar):
        if isinstance(scalar, (int,float)):
            return Vector3D(self.x * scalar, self.y * scalar, self.z * scalar, self[3])
        else:
            return NotImplemented

    def __rmul__(self, scalar):
        return self * scalar

    def __imul__(self, scalar):
        if isinstance(scalar, (int,float)):
            self.x *= scalar
            self.y *= scalar
            self.z *= scalar
            return self
        return NotImplemented


class Matrix(np.ndarray):
    def __new__(cls):
        obj = np.eye(4, dtype=float).view(cls)
        return obj