import os
import logging
from datetime import datetime, timedelta, timezone

from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa

from constants import CERTS_DIR, COUNTRY_NAME, LOCALITY_NAME, ORGANIZATION_NAME, STATE_OR_PROVINCE_NAME

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


def create_key_pair():
    """Generate a private key."""
    key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
    )
    return key

def create_self_signed_cert(key, common_name):
    """Create a self-signed root certificate."""
    subject = issuer = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "US"),
        x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, "California"),
        x509.NameAttribute(NameOID.LOCALITY_NAME, "San Francisco"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "My Organization"),
        x509.NameAttribute(NameOID.COMMON_NAME, common_name),
    ])
    cert = x509.CertificateBuilder().subject_name(
        subject
    ).issuer_name(
        issuer
    ).public_key(
        key.public_key()
    ).serial_number(
        x509.random_serial_number()
    ).not_valid_before(
        datetime.now(timezone.utc)
    ).not_valid_after(
        datetime.now(timezone.utc) + timedelta(days=365)
    ).add_extension(
        x509.BasicConstraints(ca=True, path_length=None), critical=True,
    ).sign(key, hashes.SHA256())
    return cert

def create_signed_cert(key, issuer_cert, issuer_key, common_name):
    """Create a certificate signed by the issuer."""
    subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, COUNTRY_NAME),
        x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, STATE_OR_PROVINCE_NAME),
        x509.NameAttribute(NameOID.LOCALITY_NAME, LOCALITY_NAME),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, ORGANIZATION_NAME),
        x509.NameAttribute(NameOID.COMMON_NAME, common_name),
    ])
    cert = x509.CertificateBuilder().subject_name(
        subject
    ).issuer_name(
        issuer_cert.subject
    ).public_key(
        key.public_key()
    ).serial_number(
        x509.random_serial_number()
    ).not_valid_before(
        datetime.now(timezone.utc)
    ).not_valid_after(
        datetime.now(timezone.utc) + timedelta(days=365)
    ).add_extension(
        x509.BasicConstraints(ca=False, path_length=None), critical=True,
    ).sign(issuer_key, hashes.SHA256())
    return cert

def save_key_and_cert(key, cert, key_path, cert_path):
    """Save the private key and certificate to files."""
    with open(key_path, "wb") as key_file:
        key_file.write(key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.TraditionalOpenSSL,
            encryption_algorithm=serialization.NoEncryption(),
        ))
    with open(cert_path, "wb") as cert_file:
        cert_file.write(cert.public_bytes(serialization.Encoding.PEM))

def main():
    if not os.path.exists(CERTS_DIR):
        os.makedirs(CERTS_DIR)

    root_key = create_key_pair()
    root_cert = create_self_signed_cert(root_key, "My Root CA")
    save_key_and_cert(root_key, root_cert, f"{CERTS_DIR}/root.key", f"{CERTS_DIR}/root.crt")
    logger.info(f"Root cert has been generated in the '{CERTS_DIR}' directory.")

    server_key = create_key_pair()
    server_cert = create_signed_cert(server_key, root_cert, root_key, "Server")
    save_key_and_cert(server_key, server_cert, f"{CERTS_DIR}/server.key", f"{CERTS_DIR}/server.crt")
    logger.info(f"Server cert has been generated in the '{CERTS_DIR}' directory.")

    proxy_key = create_key_pair()
    proxy_cert = create_signed_cert(proxy_key, root_cert, root_key, "Proxy")
    save_key_and_cert(proxy_key, proxy_cert, f"{CERTS_DIR}/proxy.key", f"{CERTS_DIR}/proxy.crt")
    logger.info(f"Proxy cert has been generated in the '{CERTS_DIR}' directory.")

    client_key = create_key_pair()
    client_cert = create_signed_cert(client_key, root_cert, root_key, "Client")
    save_key_and_cert(client_key, client_cert, f"{CERTS_DIR}/client.key", f"{CERTS_DIR}/client.crt")
    logger.info(f"Client cert has been generated in the '{CERTS_DIR}' directory.")

    auth_key = create_key_pair()
    auth_cert = create_signed_cert(auth_key, root_cert, root_key, "Auth Service")
    save_key_and_cert(auth_key, auth_cert, f"{CERTS_DIR}/auth.key", f"{CERTS_DIR}/auth.crt")
    logger.info(f"Auth cert has been generated in the '{CERTS_DIR}' directory.")
    logger.info(f"Certificates and keys have been generated in the '{CERTS_DIR}' directory.")

if __name__ == "__main__":
    main()
