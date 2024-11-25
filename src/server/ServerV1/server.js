const express = require('express');
const swaggerUi = require('swagger-ui-express');
const YAML = require('yamljs');
const messageRoutes = require('./app/route/messageRoutes');
const swaggerDocument = YAML.load('./api.yaml');
const app = express();
const port = 3000;


app.use(express.json());

app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerDocument));
app.use('/api', messageRoutes);

app.get('/', (req, res) => {
  res.send(`<p>API docs available at <a>http://localhost:${port}/api-docs</a></p>`);
});

// Start
app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
  console.log(`API docs available at http://localhost:${port}/api-docs`);
});
