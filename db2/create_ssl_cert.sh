#!/bin/bash
# Script to create a self-signed SSL certificate for mu3edb0
# This allows https://mu3edb0/partsdb to work without specifying port 3001

echo "Creating SSL certificate directory..."
sudo mkdir -p /etc/nginx/ssl

echo "Generating self-signed SSL certificate..."
echo "This will create a certificate valid for 365 days"
sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout /etc/nginx/ssl/mu3edb0.key \
    -out /etc/nginx/ssl/mu3edb0.crt \
    -subj "/C=CH/ST=Zurich/L=Zurich/O=PSI/OU=Mu3e/CN=mu3edb0.psi.ch"

echo "Setting proper permissions..."
sudo chmod 600 /etc/nginx/ssl/mu3edb0.key
sudo chmod 644 /etc/nginx/ssl/mu3edb0.crt
sudo chown root:root /etc/nginx/ssl/mu3edb0.*

echo ""
echo "SSL certificate created successfully!"
echo "Certificate: /etc/nginx/ssl/mu3edb0.crt"
echo "Private key: /etc/nginx/ssl/mu3edb0.key"
echo ""
echo "Next steps:"
echo "1. Update nginx configuration: sudo cp /home/mu3e/mu3eanca/db2/nginx.conf /etc/nginx/sites-available/mu3edb0.conf"
echo "2. Test configuration: sudo nginx -t"
echo "3. Reload nginx: sudo systemctl reload nginx"
echo ""
echo "Note: Browsers will show a security warning for self-signed certificates."
echo "You can accept the warning to proceed, or use proper certificates from your organization."
