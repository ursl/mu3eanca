// ----------------------------------------------------------------------
// -- uploads and downloads a single JSON file as such
// -- invocation:
//    moor>curl -X POST -F "tag=j2" -F "filename=j2/root.json" -F "file=@j2/root.json" http://localhost:3000/upload
//    moor>curl http://localhost:3000/download/j2 -o root.json
// -- the problem is that the object slows down mongodb compass quite noticably already from the start.
// ----------------------------------------------------------------------

const express = require('express');
const { MongoClient } = require('mongodb');
const multer = require('multer');
const fs = require('fs');
const app = express();

// MongoDB connection URL and database details
const url = 'mongodb://localhost:27017';
const dbName = 'mu3e';
const client = new MongoClient(url);

// Multer setup to handle file uploads
const upload = multer({ dest: 'uploads/' });

app.post('/upload', upload.single('file'), async (req, res) => {
    try {
        await client.connect();
        const db = client.db(dbName);
        const collection = db.collection('detconfigs');

        const { tag, filename } = req.body; // Get 'tag' and 'filename' from form data
        const filePath = req.file.path; // Temporary path of the uploaded file

        // Read the file contents (assuming it's a JSON file)
        const fileContent = fs.readFileSync(filePath, 'utf8');
        const jsonData = JSON.parse(fileContent);

        // Insert the document with the tag, filename, and file contents
        await collection.insertOne({
            tag,
            filename,
            content: jsonData
        });

        // Clean up the temporary file
        fs.unlinkSync(filePath);

        res.status(200).send('File uploaded successfully');
    } catch (error) {
        console.error(error);
        res.status(500).send('Error uploading file');
    } finally {
        await client.close();
    }
});

app.get('/download/:tag', async (req, res) => {
    try {
        await client.connect();
        const db = client.db("mu3e");
        const collection = db.collection('detconfigs');

        // Find the document with the given tag
        const fileDocument = await collection.findOne({ tag: req.params.tag });

        if (fileDocument) {
            // Send the JSON content as the response
            res.json(fileDocument.content);
        } else {
            res.status(404).send('File not found');
        }
    } catch (error) {
        console.error(error);
        res.status(500).send('Error retrieving file');
    } finally {
        await client.close();
    }
});



app.listen(3000, () => {
    console.log('Server is running on port 3000');
});
