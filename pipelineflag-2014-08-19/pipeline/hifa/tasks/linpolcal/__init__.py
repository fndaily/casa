from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .linpolcal import Linpolcal
from . import resultobjects

qaadapter.registry.register_to_calibration_topic(resultobjects.LinpolcalResult)
