"""
Definition of models.
"""

from django.db import models
from django.contrib.gis.db import models as gismodels

class Destination(gismodels.Model):

    id = models.IntegerField(primary_key=True)
    title = models.CharField(max_length=256)
    geom = gismodels.PointField()
    objects = gismodels.GeoManager()

    def __str__(self):
        return self.title
