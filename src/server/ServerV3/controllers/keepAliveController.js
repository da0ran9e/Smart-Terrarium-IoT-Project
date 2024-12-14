const KeepAlive = require('../models/keepAlive');  // Import the model

exports.getKeepAliveStatus = (req, res) => {
  const { id } = req.params;
  
  KeepAlive.findOne({ Id: id }).sort({ timestamp: -1 }).limit(1)
    .then((status) => {
      if (!status) {
        return res.status(404).json({ message: 'Keep-alive data not found' });
      }
      res.json(status);
    })
    .catch((error) => {
      res.status(500).json({ error: 'Server error while retrieving keep-alive data' });
    });
};
